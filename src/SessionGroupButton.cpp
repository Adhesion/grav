/*
 * @file SessionGroup.cpp
 *
 * Implementation of the session group button. See SessionGroup.h for details.
 *
 * Created on: Nov 7, 2011
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

#include "SessionGroupButton.h"
#include "SessionManager.h"

SessionGroupButton::SessionGroupButton( float _x, float _y ) :
    RectangleBase( _x, _y )
{
    userMovable = false;
    playing = false;
}

void SessionGroupButton::draw()
{
    RectangleBase::draw();

    glPushMatrix();

    glRotatef( xAngle, 1.0, 0.0, 0.0 );
    glRotatef( yAngle, 0.0, 1.0, 0.0 );
    glRotatef( zAngle, 0.0, 0.0, 1.0 );

    glTranslatef( x, y, z );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    float Xdist = scaleX / 2.5;
    float Ydist = scaleY / 2.5;

    glColor4f( borderColor.R * 0.5f, borderColor.G * 0.5f,
                borderColor.B * 0.5f, borderColor.A * 0.7f );

    if ( playing )
    {
        // the pause symbol
        glBegin( GL_QUADS );

        glVertex3f( -Xdist * 0.7f, -Ydist * 0.8f, 0.0 );
        glVertex3f( -Xdist * 0.7f, Ydist * 0.8f, 0.0 );
        glVertex3f( -Xdist * 0.25f, Ydist * 0.8f, 0.0 );
        glVertex3f( -Xdist * 0.25f, -Ydist * 0.8f, 0.0 );

        glVertex3f( Xdist * 0.7f, -Ydist * 0.8f, 0.0 );
        glVertex3f( Xdist * 0.7f, Ydist * 0.8f, 0.0 );
        glVertex3f( Xdist * 0.25f, Ydist * 0.8f, 0.0 );
        glVertex3f( Xdist * 0.25f, -Ydist * 0.8f, 0.0 );

        glEnd();
    }
    else
    {
        // the main triangle
        glBegin( GL_TRIANGLES );

        glVertex3f( -Xdist * 0.7f, -Ydist, 0.0 );
        glVertex3f( -Xdist * 0.7f, Ydist, 0.0 );
        glVertex3f( Xdist, 0.0, 0.0 );

        glEnd();
    }

    glDisable( GL_BLEND );

    glPopMatrix();
}

void SessionGroupButton::doubleClickAction()
{
    // this assumes that this object's direct parent is the SessionManager

    Group* parent = getGroup();
    if ( parent == NULL )
    {
        gravUtil::logWarning( "SessionGroupButton::doubleClick: entry not "
                "grouped? (invalid session setup)\n" );
        return;
    }

    SessionManager* manager = dynamic_cast<SessionManager*>( parent );
    if ( manager == NULL )
    {
        gravUtil::logWarning( "SessionGroupButton::doubleClick: not member of "
                "one of session manager? (invalid session setup)\n" );
        return;
    }

    manager->sessionGroupButtonAction( this );
}

void SessionGroupButton::setControlledGroup( SessionGroup* g )
{
    controlledGroup = g;
}

SessionGroup* SessionGroupButton::getControlledGroup()
{
    return controlledGroup;
}

void SessionGroupButton::setPlaying( bool p )
{
    playing = p;
}
