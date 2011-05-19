/*
 * @file VenueNode.h
 *
 * Visual representation of an Access Grid venue.
 *
 * Created on: Nov 11, 2010
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

#ifndef VENUENODE_H_
#define VENUENODE_H_

#include "RectangleBase.h"

class VenueNode : public RectangleBase
{

public:
    VenueNode();
    ~VenueNode();

    void draw();

    bool updateName();

    void doubleClickAction();

private:

};

#endif /* VENUENODE_H_ */
