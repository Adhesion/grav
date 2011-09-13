/*
 * @file SessionGroup.cpp
 *
 * Implementation of the session group. See SessionGroup.h for details.
 *
 * Created on: Sep 7, 2011
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

#include "SessionGroup.h"
#include "SessionManager.h"

SessionGroup::SessionGroup( float _x, float _y ) :
    Runway( _x, _y )
{
    rearrangeStyle = ONEROW;
}

void SessionGroup::handleOutsideMembers()
{
    std::vector<RectangleBase*> outsideList = checkMemberIntersect();
    if ( outsideList.size() > 0 )
    {
        Group* parent = getGroup();
        if ( parent == NULL )
        {
            gravUtil::logWarning( "SessionGroup::handleOutsideMembers: "
                    "no parent? invalid group setup\n" );
            return;
        }
        SessionManager* manager = dynamic_cast<SessionManager*>( parent );
        if ( manager == NULL )
        {
            gravUtil::logWarning( "SessionGroup::handleOutsideMembers: "
                    "parent not SessionManager? invalid group setup\n" );
            return;
        }
    }
}
