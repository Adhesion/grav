/*
 * @file VenueNode.cpp
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

#include "VenueNode.h"
#include "VenueClientController.h"

VenueNode::VenueNode()
{
    titleStyle = CENTEREDTEXT;
    relativeTextScale = 0.0015;
    coloredText = false;
    borderScale = 0.0f;
    userMovable = false;
    selectable = true;
    //debugDraw = true;

    baseBColor.R = 0.7f;
    baseBColor.G = 0.7f;
    baseBColor.B = 1.0f;
    baseBColor.A = 0.8f;
    destBColor = baseBColor;
    borderColor = destBColor;
    borderColor.A = 0.0f;
    setColor( destBColor );
    setPos( 0.0f, 0.0f );
    scaleX = destScaleX = 3.0f;
    scaleY = destScaleY = 3.0f;
}

void VenueNode::draw()
{
    RectangleBase::draw();
}

bool VenueNode::updateName()
{
    // like runway, this does nothing
    return false;
}

void VenueNode::doubleClickAction()
{
    // we can be pretty sure the parent is a venue client controller, if not
    // this will do nothing
    VenueClientController* vcc = dynamic_cast<VenueClientController*>(myGroup);
    if ( vcc != NULL )
    {
        vcc->enterVenue( getName() );
    }
}
