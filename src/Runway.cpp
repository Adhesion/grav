/*
 * @file Runway.cpp
 *
 * Implementation of a group object for incoming or muted videos.
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

#include "Runway.h"
#include <sstream>

Runway::Runway( float _x, float _y ) :
    Group( _x, _y )
{
    locked = false;
    selectable = false;
    userMovable = false;
    allowHiding = true;
    orientation = 1;
    setName( "Runway" );
    intersectCounter = 0;

    rearrangeStyle = ONECOLUMN;

    destBColor.R = 0.4f; destBColor.G = 0.4f;
    destBColor.B = 0.4f; destBColor.A = 0.35f;
    baseBColor = destBColor;
    borderColor.A = 0.0f;
    setColor( destBColor );
}

void Runway::draw()
{
    animateValues();

    if ( borderColor.A < 0.01f )
        return;

    // note this must set up the position itself, since it doesn't call the
    // inherited draw method from RectangleBase
    glPushMatrix();

    glRotatef( xAngle, 1.0, 0.0, 0.0 );
    glRotatef( yAngle, 0.0, 1.0, 0.0 );
    glRotatef( zAngle, 0.0, 0.0, 1.0 );

    glTranslatef( x, y, z );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    float Xdist = scaleX / 2;
    float Ydist = scaleY / 2;

    // the main box
    glBegin( GL_QUADS );

    glColor4f( borderColor.R * 0.5f, borderColor.G * 0.5f,
                borderColor.B * 0.5f, borderColor.A * 0.7f );

    glVertex3f( -Xdist, -Ydist, 0.0 );
    glVertex3f( -Xdist, Ydist, 0.0 );
    glVertex3f( Xdist, Ydist, 0.0 );
    glVertex3f( Xdist, -Ydist, 0.0 );

    glEnd();

    // the outline
    glBegin( GL_LINE_LOOP );

    glColor4f( borderColor.R, borderColor.G,
                borderColor.B, borderColor.A );

    glVertex3f( -Xdist, -Ydist, 0.0 );
    glVertex3f( -Xdist, Ydist, 0.0 );
    glVertex3f( Xdist, Ydist, 0.0 );
    glVertex3f( Xdist, -Ydist, 0.0 );

    glEnd();

    glDisable( GL_BLEND );

    glPopMatrix();

    // draw members like a normal group
    for ( unsigned int i = 0; i < objects.size(); i++ )
    {
        objects[i]->draw();
    }
}

bool Runway::updateName()
{
    // the name of the runway isn't dependent on the names of the group members,
    // so it doesn't need to be updated in that fashion
    // (return false since it doesn't change)
    return false;
}

std::vector<RectangleBase*> Runway::checkMemberIntersect()
{
    unsigned int num = objects.size();
    std::vector<RectangleBase*> outsideList;
    bool foundSelected = false;

    for ( unsigned int i = 0; i < num; i++ )
    {
        RectangleBase* obj = objects[i];
        float ox = obj->getDestX();
        float oy = obj->getDestY();

        // don't add selected objects to outside list - user might be dragging
        // them, so we don't want to interrupt that
        if ( obj->isSelected() )
        {
            foundSelected = true;
        }
        // note that this is slightly different from RectBase's intersect in
        // that it's checking the center - should help with overlap conditions
        // in SessionManager groups
        else if ( ox > getDestRBound() || ox < getDestLBound() ||
                  oy > getDestUBound() || oy < getDestDBound() )
        {
            outsideList.push_back( obj );
        }
    }

    // rearrange in the general case if none are selected or outside - accounts
    // for resetting position of objects moved internally
    if ( outsideList.size() == 0 && !foundSelected )
        rearrange();

    return outsideList;
}

void Runway::handleOutsideMembers()
{
    std::vector<RectangleBase*> outsideList = checkMemberIntersect();
    for ( int i = 0; i < outsideList.size(); i++ )
    {
        remove( outsideList[i], false );
    }

    if ( outsideList.size() > 0 )
        rearrange();
}
