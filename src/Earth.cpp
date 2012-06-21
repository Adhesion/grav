/*
 * @file Earth.h
 *
 * Defines a class for rendering a textured Earth sphere, and also provides
 * methods for converting lat/long coordinates to X/Y/Z 3d world space.
 *
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

#include "Earth.h"

#include <cmath>

Earth::Earth()
{
    x = 0.0f; y = 0.0f, z = -25.0f;
    radius = 15.0f;
    xRot = 0.0f; yRot = 0.0f; zRot = 0.0f;
    destXRot = xRot; destYRot = yRot; destZRot = zRot;
    moveAmt = 1.0f;

    Texture t = GLUtil::getInstance()->getTexture( "earth" );
    earthTex = t.ID;
    texWidth = t.width;
    texHeight = t.height;

    animated = true;
    rotating = false;

    sphereQuad = gluNewQuadric();
    gluQuadricTexture( sphereQuad, GL_TRUE );

    // create the display list for rendering the sphere
    sphereIndex = glGenLists( 1 );
    glNewList( sphereIndex, GL_COMPILE );
    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, earthTex );

    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

    gluSphere( sphereQuad, radius, 40, 40 );

    glDisable( GL_CULL_FACE );
    glDisable( GL_TEXTURE_2D );
    glEndList();

    matrix = new GLdouble[16];
}

Earth::~Earth()
{
    glDeleteTextures( 1, &earthTex );
    gluDeleteQuadric( sphereQuad );
    delete[] matrix;
    glDeleteLists( sphereIndex, 1 );
}

void Earth::draw()
{
    animateValues();

    glPushMatrix();

    glTranslatef( x, y, z );
    glRotatef( xRot-90.0f, 1.0f, 0.0f, 0.0f );
    glRotatef( yRot, 0.0f, 1.0f, 0.0f );
    glRotatef( zRot, 0.0f, 0.0f, 1.0f );

    /*glColor4f( 1.0f, 0.0f, 0.0f, 1.0f );
    glBegin( GL_LINE );
    glVertex3f( x, y, z );
    glVertex3f( x+10.0f, y, z );
    glEnd();

    glColor4f( 0.0f, 1.0f, 0.0f, 1.0f );
    glBegin( GL_LINE );
    glVertex3f( x, y, z );
    glVertex3f( x, y+10.0f, z );
    glEnd();*/

    // these show the earth axes
    /*
    glColor4f( 1.0f, 0.0f, 0.0f, 1.0f );
    glBegin( GL_LINE );
    glVertex3f( 0, 0, 0 );
    glVertex3f( x+50.0f, 0, 0 );
    glEnd();

    glColor4f( 0.0f, 1.0f, 0.0f, 1.0f );
    glBegin( GL_LINE );
    glVertex3f( 0, 0, 0 );
    glVertex3f( 0, 0+50.0f, 0 );
    glEnd();

    glColor4f( 0.0f, 0.0f, 1.0f, 1.0f );
    glBegin( GL_LINE );
    glVertex3f( 0, 0, 0 );
    glVertex3f( 0, 0, 0+50.0f );
    glEnd();
    */

    glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

    // render the sphere using its display list
    glCallList( sphereIndex );

    glPopMatrix();

    /*testLat++;
    x += moveAmt;
    if ( x > 25.0f || x < -25.0f )
        moveAmt *= -1.0f;*/
}

void Earth::convertLatLong( float lat, float lon, float &ex, float &ey,
                            float &ez, bool dest )
{
    float xr = dest ? destXRot : xRot;
    float yr = dest ? destYRot : yRot;
    float zr = dest ? destZRot : zRot;

    // grab the modelview matrix so we can calculate the result of the
    // rotation manually
    glPushMatrix();
    glLoadIdentity();
    glTranslatef( x, y, z );
    glRotatef( xr, 1.0f, 0.0f, 0.0f );
    glRotatef( yr, 0.0f, 0.0f, 1.0f );
    glRotatef( zr, 0.0f, 1.0f, 0.0f );
    glGetDoublev( GL_MODELVIEW_MATRIX, matrix );
    glPopMatrix();

    float rlat = lat;//-90.0f); //-xRot
    float rlon = lon; //+zRot

    rlat = rlat*PI/180.0f;
    rlon = rlon*PI/180.0f;

    float ext = radius * (cos(rlat) * sin(rlon));
    float eyt = radius * (sin(rlat));
    float ezt = radius * (cos(rlat) * cos(rlon));

    // column major
    ex = (ext*matrix[0]) + (eyt*matrix[4]) + (ezt*matrix[8]) + matrix[12];
    ey = (ext*matrix[1]) + (eyt*matrix[5]) + (ezt*matrix[9]) + matrix[13];
    ez = (ext*matrix[2]) + (eyt*matrix[6]) + (ezt*matrix[10]) + matrix[14];
}

Point Earth::convertLatLong( float lat, float lon, bool dest )
{
    Point p;
    float x, y, z;
    convertLatLong( lat, lon, x, y, z, dest );
    p.setX( x );
    p.setY( y );
    p.setZ( z );
    return p;
}

void Earth::rotate( float x, float y, float z )
{
    destXRot += x;
    destYRot += y;
    destZRot += z;

    if ( !animated )
    {
        xRot += x;
        yRot += y;
        zRot += z;
    }
    else
        rotating = true;
}

float Earth::getX()
{
    return x;
}

float Earth::getY()
{
    return y;
}

float Earth::getZ()
{
    return z;
}

Point Earth::getPos()
{
    return Point( x, y, z );
}

float Earth::getRadius()
{
    return radius;
}

void Earth::animateValues()
{
    // this could be a bit better (individual bools for each axis) but this
    // should be fine for now
    if ( rotating )
    {
        xRot = xRot + ( ( destXRot - xRot ) / 5.0f );
        yRot = yRot + ( ( destYRot - yRot ) / 5.0f );
        zRot = zRot + ( ( destZRot - zRot ) / 5.0f );

        if ( fabs( xRot - destXRot ) < 0.01f && fabs( yRot - destYRot ) < 0.01f
                && fabs( zRot - destZRot ) < 0.01f )
        {
            xRot = destXRot;
            yRot = destYRot;
            zRot = destZRot;
            rotating = false;
        }
    }
}
