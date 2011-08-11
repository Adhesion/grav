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

    destBColor.R = 1.0f; destBColor.G = 1.0f;
    destBColor.B = 1.0f; destBColor.A = 1.0f;
    baseBColor = destBColor;
    borderColor.A = 0.0f;
    setColor( destBColor );
}

void Runway::draw()
{
    animateValues();

    if ( borderColor.A < 0.01f )
        return;

    if ( intersectCounter == 0 && objects.size() > 0 )
        checkMemberIntersect();

    intersectCounter = ( intersectCounter + 1 ) % 10;

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

    glColor4f( borderColor.R * 0.2f, borderColor.G * 0.2f,
                borderColor.B * 0.25f, borderColor.A * 0.25f );

    glVertex3f( -Xdist, -Ydist, 0.0 );
    glVertex3f( -Xdist, Ydist, 0.0 );
    glVertex3f( Xdist, Ydist, 0.0 );
    glVertex3f( Xdist, -Ydist, 0.0 );

    glEnd();

    // the outline
    glBegin( GL_LINE_LOOP );

    glColor4f( borderColor.R * 0.4f, borderColor.G * 0.4f,
                borderColor.B * 0.45f, borderColor.A * 0.35f );

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

void Runway::checkMemberIntersect()
{
    bool removed = false;
    unsigned int num = objects.size();

    for ( unsigned int i = 0; i < num; )
    {
        RectangleBase* obj = objects[i];
        float ox = obj->getDestX();
        float oy = obj->getDestY();

        if ( ox > getRBound() || ox < getLBound() ||
                oy > getUBound() || oy < getDBound() )
        {
            remove( obj, false );
            removed = true;
            num--;
        }
        else
        {
            i++;
        }
    }

    if ( removed )
        rearrange();
}
