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

#include "SessionEntry.h"

SessionEntry::SessionEntry( std::string addr, bool aud )
{
    audio = aud;
    setName( addr );
}

SessionEntry::~SessionEntry()
{
    disableSession();
}

bool initSession()
{
    if ( !audio )
    {

    }
    else
    {

    }
}

void disableSession()
{
    if ( isSessionEnabled() )
    {
        delete session;
        session = NULL;
    }
}

bool isSessionEnabled()
{
    return ( session != NULL );
}

void SessionEntry::setProcessingEnabled( bool proc )
{
    processingEnabled = proc;
}

bool SessionEntry::getProcessingEnabled()
{
    return processingEnabled;
}

std::string SessionEntry::getAddress()
{
    return address;
}

void SessionEntry::iterate()
{
    if ( isSessionEnabled() && processingEnabled )
        session->iterate( sessionTS++ );
}
