/*
 * @file SessionManager.cpp
 *
 * Implementation of the session manager.
 *
 * Created on: Sep 28, 2010
 * @author Andrew Ford
 */

#include <VPMedia/VPMSession.h>
#include <VPMedia/VPMSessionFactory.h>
#include <VPMedia/thread_helper.h>
#include <VPMedia/random_helper.h>

#include <stdio.h>

#include "SessionManager.h"
#include "VideoListener.h"
#include "AudioManager.h"

SessionManager::SessionManager( VideoListener* vl, AudioManager* al )
    : videoSessionListener( vl ), audioSessionListener( al )
{
    sessionMutex = mutex_create();
}

SessionManager::~SessionManager()
{
    mutex_free( sessionMutex );
    for ( unsigned int i = 0; i < sessions.size(); i++ )
    {
        delete sessions[i].session;
    }
}

bool SessionManager::initSession( std::string address, bool audio )
{
    mutex_lock( sessionMutex );

    SessionEntry entry;
    VPMSession* session;
    VPMSessionFactory* factory = VPMSessionFactory::getInstance();
    std::string type = std::string( audio ? "audio" : "video" );
    int* counter = audio ? &audioSessionCount : &videoSessionCount;

    if ( !audio )
    {
        session = factory->createSession( address.c_str(),
                                        *videoSessionListener );

        session->enableVideo( true );
        session->enableAudio( false );
        session->enableOther( false );
    }
    else
    {
        session = factory->createSession( address.c_str(),
                                        *audioSessionListener);

        session->enableVideo(false);
        session->enableAudio(true);
        session->enableOther(false);
    }

    if ( !session->initialise() )
    {
        fprintf(stderr, "error: failed to initialise session\n");
        mutex_unlock( sessionMutex );
        return false;
    }

    printf( "SessionManager::initialized %s session on %s\n", type.c_str(),
                address.c_str() );
    *counter++;
    entry.sessionTS = random32();
    entry.address = address;
    entry.audio = audio;
    entry.enabled = true;
    entry.session = session;
    sessions.push_back( entry );

    mutex_unlock( sessionMutex );
    return true;
}

bool SessionManager::removeSession( std::string addr )
{
    mutex_lock( sessionMutex );

    std::vector<SessionEntry>::iterator it = sessions.begin();
    while ( (*it).address.compare( addr ) != 0 ) it++;
    if ( it == sessions.end() )
    {
        mutex_unlock( sessionMutex );
        return false;
    }

    int* counter = (*it).audio ? &audioSessionCount : &videoSessionCount;
    (*counter)--;
    delete (*it).session;
    sessions.erase( it );
    mutex_unlock( sessionMutex );
    return true;
}

bool SessionManager::setSessionEnable( std::string addr, bool set )
{
    mutex_lock( sessionMutex );

    std::vector<SessionEntry>::iterator it = sessions.begin();
    while ( (*it).address.compare( addr ) != 0 ) it++;
    if ( it == sessions.end() )
    {
        mutex_unlock( sessionMutex );
        return false;
    }

    (*it).enabled = set;
    mutex_unlock( sessionMutex );
    return true;
}

bool SessionManager::isSessionEnabled( std::string addr )
{
    mutex_lock( sessionMutex );

    std::vector<SessionEntry>::iterator it = sessions.begin();
    while ( (*it).address.compare( addr ) != 0 ) it++;
    if ( it == sessions.end() )
    {
        mutex_unlock( sessionMutex );
        return false;
    }

    bool ret = (*it).enabled;
    mutex_unlock( sessionMutex );
    return ret;
}

bool SessionManager::iterateSessions()
{
    mutex_lock( sessionMutex );

    bool haveSessions = false;
    for ( unsigned int i = 0; i < sessions.size(); i++ )
    {
        if ( sessions[i].enabled )
            sessions[i].session->iterate( sessions[i].sessionTS++ );
        haveSessions = haveSessions || sessions[i].enabled;
    }

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
