/*
 * VenueNode.cpp
 *
 * Visual representation of an Access Grid venue.
 *
 * Created on: Nov 11, 2010
 * @author Andrew Ford
 */

#include "VenueNode.h"

VenueNode::VenueNode()
{
    titleStyle = CENTEREDTEXT;
    relativeTextScale = 0.0015;
    borderScale = 0.0f;
    userMovable = false;
    selectable = true;
    //debugDraw = true;

    baseBColor.R = 0.7f;
    baseBColor.G = 0.7f;
    baseBColor.B = 1.0f;
    baseBColor.A = 0.8f;
    destBColor = baseBColor;
    destBColor.A = 0.0f;
    borderColor = destBColor;
}

void VenueNode::draw()
{
    //printf( "VN name: %s\n", name.c_str() );
    RectangleBase::draw();
}

bool VenueNode::updateName()
{
    // like runway, this does nothing
    return false;
}
