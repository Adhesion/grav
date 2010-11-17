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
    borderScale = 0.0f;
    userMovable = false;
    selectable = true;
    //debugDraw = true;
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
