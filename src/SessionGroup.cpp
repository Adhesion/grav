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
#include "SessionGroupButton.h"
#include "GLCanvas.h"
#include "Timers.h"

SessionGroup::SessionGroup( float _x, float _y ) :
    Runway( _x, _y )
{
    rearrangeStyle = ONEROW;
    // note this inherits unlocked, unselectable, and unmovable from runway

    rotating = false;
    timer = NULL;
}

void SessionGroup::handleOutsideMembers()
{
    // do nothing here - parent (SessionManager) will have its check method
    // called, which will get the list of outside members and figure out where
    // they should go (ie, shouldn't necessarily remove like runway does)
}

void SessionGroup::draw()
{
    // note this duplicates runway's draw so we can stick the rotating animation
    // in between the border/background and the member drawing
    animateValues();

    drawRunwayBorder();

    if ( rotating && timer != NULL )
    {
        float xPos = getLBound() + ( scaleX * timer->getProgress() );
        float size = scaleY / 4.0f;

        glPushMatrix();

        glRotatef( xAngle, 1.0, 0.0, 0.0 );
        glRotatef( yAngle, 0.0, 1.0, 0.0 );
        glRotatef( zAngle, 0.0, 0.0, 1.0 );

        glTranslatef( xPos, y, z );

        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

        glColor4f( borderColor.R, borderColor.G, borderColor.B, borderColor.A );

        glBegin( GL_TRIANGLES );

        glVertex3f( -size * 0.7f, -size, 0.0 );
        glVertex3f( -size * 0.7f, size, 0.0 );
        glVertex3f( size, 0.0, 0.0 );

        glEnd();

        glDisable( GL_BLEND );

        glPopMatrix();
    }

    // draw members like a normal group
    for ( unsigned int i = 0; i < objects.size(); i++ )
    {
        objects[i]->draw();
    }
}

bool SessionGroup::isRotating()
{
    return rotating;
}

void SessionGroup::setRotating( bool r )
{
    rotating = r;
}

void SessionGroup::setTimer( RotateTimer* t )
{
    timer = t;
}
