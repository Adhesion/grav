/*
 * Runway.cpp
 *
 *  Created on: Apr 28, 2010
 *      Author: andrew
 */

#include "Runway.h"

Runway::Runway( float _x, float _y ) :
    Group( _x, _y )
{
    locked = false;
    selectable = false;
    orientation = 1;
    setName( "Runway" );
    intersectCounter = 0;
}

void Runway::draw()
{
    if ( intersectCounter == 0 && objects.size() > 0 )
        checkMemberIntersect();

    intersectCounter = ( intersectCounter + 1 ) % 10;

    animateValues();

    // note this must set up the position itself, since it doesn't call the
    // inherited draw method from RectangleBase
    glPushMatrix();

    glRotatef( angle, 0.0, 1.0, 0.0 );
    glTranslatef( x, y, z );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    float Xdist = scaleX / 2;
    float Ydist = scaleY / 2;

    // the main box
    glBegin( GL_QUADS );

    glColor4f( 0.2f, 0.2f, 0.25f, 0.25f );

    glVertex3f( -Xdist, -Ydist, 0.0 );
    glVertex3f( -Xdist, Ydist, 0.0 );
    glVertex3f( Xdist, Ydist, 0.0 );
    glVertex3f( Xdist, -Ydist, 0.0 );

    glEnd();

    // the outline
    glBegin( GL_LINE_LOOP );

    glColor4f( 0.4f, 0.4f, 0.45f, 0.35f );

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

void Runway::rearrange()
{
    if ( objects.size() == 0 ) return;

    // horizontal
    if ( orientation == 0 )
    {
        layouts.gridArrange( getLBound(), getRBound(), getUBound(), getDBound(),
                                true, false, true, objects,
                                objects.size(), 1 );
    }
    // vertical
    else if ( orientation == 1 )
    {
        layouts.gridArrange( getLBound(), getRBound(), getUBound(), getDBound(),
                                false, false, true, objects,
                                1, objects.size() );
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
