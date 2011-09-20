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
    // do nothing here - parent (SessionManager) will have its check method
    // called, which will get the list of outside members and figure out where
    // they should go (ie, shouldn't necessarily remove like runway does)
}
