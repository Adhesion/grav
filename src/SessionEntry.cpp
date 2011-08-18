/*
 * @file SessionEntry.cpp
 *
 * Implementation of the session entry. See SessionEntry.h for details.
 *
 * Created on: Aug 11, 2011
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

#include <VPMedia/VPMSession.h>
#include <VPMedia/VPMSessionFactory.h>

#include "SessionEntry.h"

SessionEntry::SessionEntry( std::string addr, bool aud )
{
    audio = aud;
    setName( addr );

    processingEnabled = true;

    encryptionKey = "__NO_KEY__";
    encryptionEnabled = false;

    session = NULL;
}

SessionEntry::~SessionEntry()
{
    disableSession();
}

bool SessionEntry::initSession( VPMSessionListener* listener )
{
    if ( !isSessionEnabled() )
    {
        VPMSessionFactory* factory = VPMSessionFactory::getInstance();
        session = factory->createSession( address.c_str(), *listener );

        session->enableVideo( !audio );
        session->enableAudio( audio );
        session->enableOther( false );

        if ( encryptionKey.compare( "__NO_KEY__" ) != 0 )
        {
            session->setEncryptionKey( key.c_str() );
        }

        sessionTS = random32();
    }
    else
        gravUtil::logWarning( "SessionEntry::init: session %s already "
                                "initialized\n", address.c_str() );
}

void SessionEntry::disableSession()
{
    if ( isSessionEnabled() )
    {
        delete session;
        session = NULL;
    }
}

bool SessionEntry::isSessionEnabled()
{
    return ( session != NULL );
}

void SessionEntry::setProcessingEnabled( bool proc )
{
    processingEnabled = proc;
}

bool SessionEntry::isProcessingEnabled()
{
    return processingEnabled;
}

bool SessionEntry::isAudioSession()
{
    return audio;
}

void SessionEntry::setEncryptionKey( std::string key )
{
    encryptionKey = key;
    encryptionEnabled = true;
    if ( isSessionEnabled() )
        session->setEncryptionKey( key.c_str() );
    // if not enabled, should set on init
}

void SessionEntry::disableEncryption()
{
    encryptionKey = "__NO_KEY__";
    encryptionEnabled = false;
    if ( isSessionEnabled() )
        session->setEncryptionKey( NULL );
    // if not enabled, doesn't really matter since a new session will start with
    // no encryption
}

bool SessionEntry::isEncryptionEnabled()
{
    return encryptionEnabled;
}

std::string SessionEntry::getAddress()
{
    return address;
}

void SessionEntry::iterate()
{
    bool running = isSessionEnabled() && processingEnabled;
    if ( running )
        session->iterate( sessionTS++ );
    return running;
}
