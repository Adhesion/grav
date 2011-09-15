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
#include <VPMedia/random_helper.h>

#include "SessionEntry.h"
#include "Group.h"
#include "SessionManager.h"

SessionEntry::SessionEntry( std::string addr, bool aud )
{
    audio = aud;
    address = addr;
    setName( addr );

    processingEnabled = true;
    initialized = false;

    encryptionKey = "__NO_KEY__";
    encryptionEnabled = false;

    session = NULL;

    disabledColor.R = 0.15f;
    disabledColor.G = 0.15f;
    disabledColor.B = 1.0f;
    disabledColor.A = 0.55f;

    baseBColor = disabledColor;
    borderColor = disabledColor;
    borderColor.A = 0.0f;
    setColor( disabledColor );
}

SessionEntry::~SessionEntry()
{
    gravUtil::logVerbose( "~SessionEntry\n" );
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

        if ( !session->initialise() )
        {
            gravUtil::logError( "SessionEntry::init: failed to initialize on "
                                "address %s\n", address.c_str() );
            initialized = false;
            // might as well delete the session object here to prevent potential
            // memleaks (ie, reinitializing a session that failed to init)
            disableSession();
            return false;
        }

        if ( encryptionKey.compare( "__NO_KEY__" ) != 0 )
        {
            session->setEncryptionKey( encryptionKey.c_str() );
        }

        sessionTS = random32();

        initialized = true;
        resetColor();
        return true;
    }
    else
    {
        gravUtil::logWarning( "SessionEntry::init: session %s already "
                                "initialized\n", address.c_str() );
        return false;
    }
}

void SessionEntry::disableSession()
{
    // even if it failed to initialize, we still need to delete the session
    // object
    if ( session != NULL )
    {
        gravUtil::logVerbose( "SessionEntry::disableSession: deleting "
                                "VPMSession object for %s\n", address.c_str() );
        delete session;
        session = NULL;
        setBaseColor( disabledColor );
    }
    else
    {
        gravUtil::logVerbose( "SessionEntry::disableSession: session (%s) not "
                                "active, not deleting\n", address.c_str() );
    }
}

bool SessionEntry::isSessionEnabled()
{
    return ( session != NULL && initialized );
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

uint32_t SessionEntry::getTimestamp()
{
    return sessionTS;
}

bool SessionEntry::iterate()
{
    bool running = isSessionEnabled() && processingEnabled;
    if ( running )
        session->iterate( sessionTS++ );
    return running;
}

void SessionEntry::doubleClickAction()
{
    Group* parent = getGroup();
    if ( parent == NULL )
    {
        gravUtil::logWarning( "SessionEntry::doubleClick: entry not grouped? "
                "(invalid session setup)\n" );
        return;
    }

    Group* gParent = parent->getGroup();
    if ( gParent == NULL )
    {
        gravUtil::logWarning( "SessionEntry::doubleClick: parent not grouped? "
                "(invalid session setup)\n" );
        return;
    }

    SessionManager* manager = dynamic_cast<SessionManager*>( gParent );
    if ( manager == NULL )
    {
        gravUtil::logWarning( "SessionEntry::doubleClick: not member of one of "
                "session manager's groups? (invalid session setup)\n" );
        return;
    }

    manager->sessionEntryAction( this );
}
