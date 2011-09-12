/*
 * @file SessionGroup.h
 *
 * Definition of a class to group SessionEntries. (ie video sessions, audio
 * sessions, available video sessions)
 * Shares draw & intersect checking functionality with Runway which is why it
 * inherits from it.
 *
 * Created on: Sep 07, 2011
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

#ifndef SESSIONGROUP_H_
#define SESSIONGROUP_H_

#include "Runway.h"

class SessionGroup : public Runway
{

public:
    SessionGroup( float _x, float _y );

    void handleOutsideMembers();

};

#endif /* SESSIONGROUP_H_ */
