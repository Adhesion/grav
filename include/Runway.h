/*
 * @file Runway.h
 *
 * A group object that holds incoming objects before the user arranges them
 * themselves.
 *
 * Created on: Apr 28, 2010
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

#ifndef RUNWAY_H_
#define RUNWAY_H_

#include "Group.h"

class Runway : public Group
{

public:
    Runway( float _x, float _y );

    void draw();

    bool updateName();

    // check the positions of all members and remove ones that are not inside
    // the runway bounds
    void checkMemberIntersect();

private:
    // 0 means horizontal, 1 means vertical
    int orientation;

    // only check member object intersect every few frames
    int intersectCounter;

};

#endif /* RUNWAY_H_ */
