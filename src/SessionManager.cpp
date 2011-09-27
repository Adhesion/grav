/*
 * @file SessionManager.cpp
 *
 * Implementation of the session manager.
 *
 * Created on: Sep 28, 2010
 * @author Andrew Ford
 * Copyright (C) 2011 Rochester Institute of Technology
 *
 * This file is part of grav.
 *
 * grav is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * grav is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with grav.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <VPMedia/thread_helper.h>

#include <wx/utils.h>

#include <stdio.h>

#include "SessionManager.h"
#include "SessionEntry.h"
#include "SessionGroup.h"
#include "VideoListener.h"
#include "AudioManager.h"
#include "grav.h"
#include "gravUtil.h"
#include "SessionTreeControl.h"
#include "gravManager.h"

SessionManager::SessionManager( VideoListener* vl, AudioManager* al,
                                gravManager* g )
    : Group( 0.0f, -6.0f ), videoSessionListener( vl ),
      audioSessionListener( al ), objectManager( g )
{
    sessionMutex = mutex_create();

    videoSessionCount = 0;
    audioSessionCount = 0;
    lockCount = 0;
    pause = false;

    rotatePos = -1;

    preserveChildAspect = false;

    locked = true;
    selectable = false;
    userMovable = false;

    setBorderScale( 0.005 );

    RGBAColor mainColor;
    mainColor.R = 0.1f;
    mainColor.G = 0.1f;
    mainColor.B = 0.1f;
    mainColor.A = 0.3f;
    setColor( mainColor );

    videoSessions = new SessionGroup( getDestX(), getDestY() );
    videoSessions->setName( "Video" );
    videoSessions->setBorderScale( 0.005 );
    RGBAColor videoColor;
    videoColor.R = 0.1f;
    videoColor.G = 0.8f;
    videoColor.B = 0.1f;
    videoColor.A = 0.3f;
    videoSessions->setColor( videoColor );

    availableVideoSessions = new SessionGroup( getDestX(), getDestY() );
    availableVideoSessions->setName( "Available Video" );
    availableVideoSessions->setBorderScale( 0.005 );
    RGBAColor availableVideoColor;
    availableVideoColor.R = 0.1f;
    availableVideoColor.G = 0.1f;
    availableVideoColor.B = 0.8f;
    availableVideoColor.A = 0.3f;
    availableVideoSessions->setColor( availableVideoColor );

    audioSessions = new SessionGroup( getDestX(), getDestY() );
    audioSessions->setName( "Audio" );
    audioSessions->setBorderScale( 0.005 );
    RGBAColor audioColor;
    audioColor.R = 0.8f;
    audioColor.G = 0.1f;
    audioColor.B = 0.1f;
    audioColor.A = 0.3f;
    audioSessions->setColor( audioColor );

    add( videoSessions );
    add( availableVideoSessions );
    add( audioSessions );
    objectManager->addToDrawList( videoSessions );
    objectManager->addToDrawList( availableVideoSessions );
    objectManager->addToDrawList( audioSessions );

    sessionMap[ VIDEOSESSION ] = videoSessions;
    sessionMap[ AVAILABLEVIDEOSESSION ] = availableVideoSessions;
    sessionMap[ AUDIOSESSION ] = audioSessions;
}

SessionManager::~SessionManager()
{
    mutex_free( sessionMutex );

    Group* sessions;
    SessionEntry* session;
    std::vector<RectangleBase*>::iterator groupIt = getBeginIterator();
    while ( groupIt != getEndIterator() )
    {
        sessions = static_cast<Group*>( *groupIt );
        std::vector<RectangleBase*>::iterator sessionIt =
                sessions->getBeginIterator();
        while ( sessionIt != sessions->getEndIterator() )
        {
            // no need to do a static cast here since we don't use methods
            // specific to sessionentry
            RectangleBase* session = *sessionIt;
            sessionIt = sessions->remove( sessionIt );
            objectManager->removeFromLists( session, false );
            delete session;
        }
        groupIt = remove( groupIt );
        objectManager->removeFromLists( sessions, false );
        delete sessions;
    }
}

bool SessionManager::addSession( std::string address, SessionType type )
{
    lockSessions();

    bool ret = true;
    bool audio = ( type == AUDIOSESSION );
    SessionEntry* entry = new SessionEntry( address, audio );
    Group* sessions = sessionMap[ type ];

    if ( type != AVAILABLEVIDEOSESSION )
    {
        ret = ret && initSession( entry );
    }

    /*
     * Note - used to delete entries on fail. Now will display an entry in a
     * failed state, so it can potentially be reenabled when it could work.
     * (But still returns false on failed init)
     */

    sessions->add( entry );
    objectManager->addToDrawList( entry );

    unlockSessions();
    return ret;
}

bool SessionManager::removeSession( std::string addr, SessionType type )
{
    lockSessions();

    SessionEntry* entry = findSessionByAddress( addr, type );
    if ( entry == NULL )
    {
        unlockSessions();
        gravUtil::logWarning( "SessionManager::removeSession: "
                                "session %s not found\n", addr.c_str() );
        return false;
    }

    if ( type == AVAILABLEVIDEOSESSION )
    {
        int i = indexOf( entry );
        // shift rotate position back if what we're removing is before or at it,
        // so we don't skip any
        if ( i <= rotatePos && i != -1 )
            rotatePos--;
    }

    objectManager->removeFromLists( entry, false );
    delete entry; //destructor will remove object from its group
    unlockSessions();
    return true;
}

bool SessionManager::shiftSession( std::string addr, SessionType fromType )
{
    lockSessions();

    if ( fromType != VIDEOSESSION && fromType != AVAILABLEVIDEOSESSION )
    {
        gravUtil::logError( "SessionManager::shiftSession: invalid SessionType "
                            "input\n" );
        unlockSessions();
        return false;
    }

    SessionEntry* entry = findSessionByAddress( addr, fromType );
    if ( entry == NULL )
    {
        gravUtil::logError( "SessionManager::shiftSession: address %s not "
                            "found\n", addr.c_str() );
        unlockSessions();
        return false;
    }

    bool ret = shiftSession( entry );

    unlockSessions();
    return true;
}

void SessionManager::rotate( bool audio )
{
    rotateTo( "", audio );
}

void SessionManager::rotateTo( std::string addr, bool audio )
{
    lockSessions();

    int numSessions = availableVideoSessions->numObjects();
    int lastRotatePos = rotatePos;
    if ( lastRotatePos != -1 )
        lastRotateSession =
           static_cast<SessionEntry*>( (*availableVideoSessions)[ rotatePos ] );
    if ( numSessions == 0 )
    {
        unlockSessions();
        return;
    }

    SessionEntry* current;

    // if arg is an empty string, just rotate to next. otherwise, figure out
    // rotate pos of string arg
    if ( addr.compare( "" ) == 0 )
    {
        if ( ++rotatePos >= numSessions )
        {
            rotatePos = 0;
        }
        current =
           static_cast<SessionEntry*>( (*availableVideoSessions)[ rotatePos ] );
    }
    else
    {
        current = findSessionByAddress( addr, AVAILABLEVIDEOSESSION );

        if ( current == NULL )
        {
            gravUtil::logWarning( "SessionManager::rotateTo: session %s"
                    " not found\n", addr.c_str() );
            unlockSessions();
            return;
        }
        else
        {
            rotatePos = indexOf( current );
        }
    }

    // only rotate if there is a valid old one & it isn't the same as
    // current
    if ( lastRotateSession != NULL && current != NULL &&
            lastRotateSession != current )
    {
        disableSession( lastRotateSession );
        initSession( current );
    }
    // case for first rotate
    else if ( lastRotatePos == -1 )
    {
        initSession( current );
    }

    unlockSessions();
}

void SessionManager::unrotate( bool audio )
{
    lockSessions();

    SessionEntry* current = NULL;
    if ( rotatePos >= 0 && rotatePos < availableVideoSessions->numObjects() )
        current =
           static_cast<SessionEntry*>( (*availableVideoSessions)[ rotatePos ] );

    rotatePos = -1;

    lastRotateSession = NULL;

    if ( current != NULL )
    {
        disableSession( current );
    }

    unlockSessions();
}

void SessionManager::sessionEntryAction( SessionEntry* entry )
{
    // since this doesn't modify any of the lists of sessions, it should be safe
    // not to do mutex locking. (otherwise, it would just lock, do these ifs,
    // then have to unlock and relock around the rotate since it eventually
    // locks itself.) this may change in the future

    Group* group = entry->getGroup();
    if ( group == videoSessions )
    {
        // do nothing here for now
    }
    else if ( group == availableVideoSessions )
    {
        sessionTree->rotateToVideoSession( entry->getAddress() );
    }
    else if ( group == audioSessions )
    {
        // do nothing here for now
    }
    else
    {
        gravUtil::logWarning( "SessionManager::sessionEntryAction: entry not "
                                "grouped?\n" );
    }

    // TODO this makes it so the user doesn't move sessions around after
    // potentially rotating them. probably should make this not so blunt
    objectManager->clearSelected();
}

void SessionManager::checkGUISessionShift()
{
    // this method doesn't do mutex locking - similar reasons to
    // sessionEntryAction(), see above for details

    Group* target;
    SessionGroup* parent = videoSessions;

    // just loop over video & available video for now
    for ( unsigned int i = 0; i < 2; i++ )
    {
        if ( parent == videoSessions )
        {
            target = availableVideoSessions;
        }
        else if ( parent == availableVideoSessions )
        {
            target = videoSessions;
        }

        std::vector<RectangleBase*> outsideList =
                parent->checkMemberIntersect();

        for ( unsigned int i = 0; i < outsideList.size(); i++ )
        {
            SessionEntry* entry = dynamic_cast<SessionEntry*>( outsideList[i] );
            if ( entry == NULL )
            {
                gravUtil::logWarning( "SessionManager::checkGUISessionShift: "
                                        "invalid SessionGroup child?\n" );
            }
            else
            {
                // check intersect with projected destination, shift if so
                if ( entry->intersect( target ) )
                {
                    // we have to use the external method here to ensure that
                    // the side window GUI stays accurate - that will in turn
                    // call the shiftSession() method in this class
                    sessionTree->shiftSession( entry->getAddress(), false );
                }
            }
        }

        parent = availableVideoSessions;
    }
}

std::string SessionManager::getCurrentRotateSessionAddress()
{
    lockSessions();

    if ( rotatePos != -1 && rotatePos < availableVideoSessions->numObjects() )
    {
        SessionEntry* entry = static_cast<SessionEntry*>(
                (*availableVideoSessions)[ rotatePos ] );
        if ( entry != NULL )
        {
            unlockSessions();
            return entry->getAddress();
        }
    }

    gravUtil::logVerbose( "SessionManager::getCurrentRotateSessionAddress: "
                            "failed to find valid session (rotate position %i)",
                            rotatePos );
    unlockSessions();
    return "";
}

std::string SessionManager::getLastRotateSessionAddress()
{
    if ( lastRotateSession != NULL )
        return lastRotateSession->getAddress();
    else
        return "";
}

bool SessionManager::setSessionProcessEnable( std::string addr, bool set )
{
    lockSessions();

    SessionEntry* entry = findSessionByAddress( addr );
    if ( entry == NULL )
    {
        unlockSessions();
        return false;
    }

    entry->setProcessingEnabled( set );
    unlockSessions();
    return true;
}

bool SessionManager::isSessionProcessEnabled( std::string addr )
{
    lockSessions();

    SessionEntry* entry = findSessionByAddress( addr );
    if ( entry == NULL )
    {
        unlockSessions();
        return false;
    }

    bool ret = entry->isProcessingEnabled();
    unlockSessions();
    return ret;
}

bool SessionManager::isInFailedState( std::string addr, SessionType type )
{
    lockSessions();

    SessionEntry* entry = findSessionByAddress( addr, type );
    if ( entry == NULL )
    {
        unlockSessions();
        return false;
    }

    bool ret = entry->isInFailedState();
    unlockSessions();
    return ret;
}

bool SessionManager::setEncryptionKey( std::string addr, std::string key )
{
    lockSessions();

    SessionEntry* entry = findSessionByAddress( addr );
    if ( entry == NULL )
    {
        unlockSessions();
        return false;
    }

    entry->setEncryptionKey( key );

    unlockSessions();
    return true;
}

bool SessionManager::disableEncryption( std::string addr )
{
    lockSessions();

    SessionEntry* entry = findSessionByAddress( addr );
    if ( entry == NULL )
    {
        unlockSessions();
        return false;
    }

    entry->disableEncryption();

    unlockSessions();
    return true;
}

bool SessionManager::isEncryptionEnabled( std::string addr )
{
    lockSessions();

    SessionEntry* entry = findSessionByAddress( addr );
    if ( entry == NULL )
    {
        unlockSessions();
        return false; // this doesn't quite make sense - should throw some other
                      // kind of error for session not found?
    }

    bool ret = entry->isEncryptionEnabled();
    unlockSessions();
    return ret;
}

bool SessionManager::iterateSessions()
{
    // kind of a hack to force this to wait, when removing etc.
    // mutex should do this but this thread seems way too eager
    if ( pause )
    {
        gravUtil::logVerbose( "Sessions temporarily paused...\n" );
        wxMicroSleep( 10 );
    }

    // note: iterate doesn't do lockSessions() since it shouldn't affect pause
    mutex_lock( sessionMutex );
    lockCount++;

    bool haveSessions = false;
    Group* sessions;
    SessionEntry* session;
    for ( int i = 0; i < numObjects(); i++ )
    {
        sessions = static_cast<Group*>( (*this)[i] );
        for ( int j = 0; j < sessions->numObjects(); j++ )
        {
            session = static_cast<SessionEntry*>( (*sessions)[j] );
            haveSessions = session->iterate() || haveSessions;
        }

        if ( i == 0 && haveSessions && gravApp::threadDebug )
        {
            if ( session->getTimestamp() % 1000 == 0 )
            {
                gravUtil::logVerbose( "SessionManager::iterate: "
                        "have %u video sessions, last TS=%u\n",
                        sessions->numObjects(), session->getTimestamp() );
            }
        }
    }

    lockCount--;
    mutex_unlock( sessionMutex );

    return haveSessions;
}

int SessionManager::getVideoSessionCount()
{
    return videoSessionCount;
}

int SessionManager::getAudioSessionCount()
{
    return audioSessionCount;
}

void SessionManager::lockSessions()
{
    pause = true;
    mutex_lock( sessionMutex );
    lockCount++;
}

void SessionManager::unlockSessions()
{
    pause = false;
    lockCount--;
    mutex_unlock( sessionMutex );
}

void SessionManager::setSessionTreeControl( SessionTreeControl* s )
{
    sessionTree = s;
}

/*
 * Note all the following functions are private and NOT thread-safe.
 */

bool SessionManager::initSession( SessionEntry* session )
{
    bool audio = session->isAudioSession();
    std::string type = std::string( audio ? "audio" : "video" );
    VPMSessionListener* listener = audio ?
        (VPMSessionListener*)audioSessionListener :
            (VPMSessionListener*)videoSessionListener;

    if ( !session->initSession( listener ) )
    {
        gravUtil::logError( "SessionManager::initSession: "
            "failed to initialize %s session on %s\n", type.c_str(),
                session->getAddress().c_str() );
        return false;
    }

    gravUtil::logVerbose( "SessionManager::initialized %s session on %s\n",
            type.c_str(), session->getAddress().c_str() );
    return true;
}

void SessionManager::disableSession( SessionEntry* session )
{
    session->disableSession();
}

SessionEntry* SessionManager::findSessionByAddress( std::string address )
{
    Group* sessions;
    SessionEntry* session;

    for ( int i = 0; i < numObjects(); i++ )
    {
        sessions = static_cast<Group*>( (*this)[i] );
        for ( int j = 0; j < sessions->numObjects(); j++ )
        {
            session = static_cast<SessionEntry*>( (*sessions)[j] );
            if ( session->getAddress().compare( address ) == 0 )
                return session;
        }
    }

    gravUtil::logWarning( "SessionManager::findSessionByAddress: session %s "
                            "not found\n", address.c_str() );
    return NULL;
}

SessionEntry* SessionManager::findSessionByAddress( std::string address,
        SessionType type )
{
    Group* sessions = sessionMap[ type ];
    SessionEntry* session;

    for ( int j = 0; j < sessions->numObjects(); j++ )
    {
        session = static_cast<SessionEntry*>( (*sessions)[j] );
        if ( session->getAddress().compare( address ) == 0 )
            return session;
    }

    gravUtil::logWarning( "SessionManager::findSessionByAddress: session %s "
                            "not found\n", address.c_str() );
    return NULL;
}

int SessionManager::indexOf( SessionEntry* entry )
{
    Group* sessions = entry->getGroup();
    if ( sessions == NULL )
        return -1;

    int i = 0;
    while ( entry != (*sessions)[i] && i < sessions->numObjects() ) i++;
    if ( i == sessions->numObjects() )
        return -1;
    else
        return i;
}

bool SessionManager::shiftSession( SessionEntry* entry )
{
    Group* to;
    Group* from = entry->getGroup();

    if ( from == videoSessions )
    {
        to = availableVideoSessions;
        disableSession( entry );
    }
    else if ( from = availableVideoSessions )
    {
        to = videoSessions;

        if ( !entry->isSessionEnabled() )
        {
            initSession( entry );
        }

        int i = indexOf( entry );
        // shift rotate position back if what we're removing is before or at it,
        // so we don't skip any
        if ( i <= rotatePos && i != -1 )
            rotatePos--;
    }
    else
    {
        const char* name = from != NULL ? from->getName().c_str() : "NULL";
        gravUtil::logWarning( "SessionManager::shiftSession: invalid source "
                                "group input (%s)\n", name );
        return false;
    }

    from->remove( entry );
    to->add( entry );
    return true;
}
