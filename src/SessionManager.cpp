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
#include "VideoListener.h"
#include "AudioManager.h"
#include "grav.h"
#include "gravUtil.h"

SessionManager::SessionManager( VideoListener* vl, AudioManager* al )
    : Group( 0.0f, -8.0f), videoSessionListener( vl ),
      audioSessionListener( al )
{
    sessionMutex = mutex_create();

    videoSessionCount = 0;
    audioSessionCount = 0;
    lockCount = 0;
    pause = false;

    rotatePos = -1;

    videoSessions = new Group( getDestX(), getDestY() );
    availableVideoSessions = new Group( getDestX(), getDestY() );
    audioSessions = new Group( getDestX(), getDestY() );

    add( videoSessions );
    add( availableVideoSessions );
    add( audioSessions );

    sessionMap[ VIDEOSESSION ] = videoSessions;
    sessionMap[ AVAILABLEVIDEOSESSION ] = availableVideoSessions;
    sessionMap[ AUDIOSESSION ] = audioSessions;
}

SessionManager::~SessionManager()
{
    mutex_free( sessionMutex );

    Group* sessions;
    SessionEntry* session;
    for ( int i = 0; i < numObjects(); i++ )
    {
        sessions = static_cast<Group*>( (*this)[i] );
        for ( int j = 0; j < sessions->numObjects(); j++ )
        {
            session = static_cast<SessionEntry*>( (*sessions)[j] );
            delete session;
        }
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
    sessions->add( entry );

    if ( type != AVAILABLEVIDEOSESSION )
        ret = ret && initSession( entry );

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
        int i = indexOf( entry, type );
        // shift rotate position back if what we're removing is before or at it,
        // so we don't skip any
        if ( i <= rotatePos )
            rotatePos--;
    }

    delete entry; //destructor will remove object from its group
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
            rotatePos = indexOf( current, AVAILABLEVIDEOSESSION );
        }
    }

    // only remove & rotate if there is a valid old one & it isn't the same as
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

int SessionManager::indexOf( SessionEntry* entry, SessionType type )
{
    Group* sessions = sessionMap[ type ];
    int i = 0;
    while ( entry != (*sessions)[i] && i < sessions->numObjects() ) i++;
    if ( i == sessions->numObjects() )
        return -1;
    else
        return i;
}

std::string SessionManager::getCurrentRotateSessionAddress()
{
    if ( rotatePos != -1 && rotatePos < availableVideoSessions->numObjects() )
    {
        SessionEntry* entry = static_cast<SessionEntry*>(
                (*availableVideoSessions)[ rotatePos ] );
        return entry->getAddress();
    }
    else
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
        //gravUtil::logVerbose( "Sessions temporarily paused...\n" );
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
            haveSessions = haveSessions || session->iterate();
        }

        if ( i == 0 && haveSessions && gravApp::threadDebug )
        {
            if ( session->getTimestamp() % 1000 == 0 )
            {
                gravUtil::logVerbose( "SessionManager::iterate: "
                        "have %u sessions, 0th TS=%u\n",
                        sessions->numObjects(), session->getTimestamp() );
            }
        }
    }

    mutex_unlock( sessionMutex );
    lockCount--;
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
    mutex_unlock( sessionMutex );
    lockCount--;
}

/*
 * Again note these two are NOT thread-safe.
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
            "failed to initialise %s session on %s\n", type.c_str(),
                session->getAddress().c_str() );
        unlockSessions();
        return false;
    }

    gravUtil::logVerbose( "SessionManager::initialized %s session on %s\n",
            type.c_str(), session->getAddress().c_str() );
}

void SessionManager::disableSession( SessionEntry* session )
{
    session->disableSession();
}
