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
#include "gravUtil.h"
#include "PNGLoader.h"

#include <cmath>

const float PI = 3.1415926535;

Earth::Earth()
{
    x = 0.0f; y = 0.0f, z = -25.0f;
    radius = 15.0f;
    xRot = 0.0f; yRot = 0.0f; zRot = 0.0f;
    destXRot = xRot; destYRot = yRot; destZRot = zRot;
    moveAmt = 1.0f;

    earthTex = 0;
    gravUtil* util = gravUtil::getInstance();
    std::string earthTexLoc = util->findFile( "earth.png" );
    if ( earthTexLoc.compare( "" ) != 0 )
    {
        earthTex = PNGLoader::loadPNG( earthTexLoc, texWidth, texHeight );
    }
    else
    {
        gravUtil::logWarning( "Earth::init: earth texture not found\n" );
    }

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
                            float &ez)
{
    // grab the modelview matrix so we can calculate the result of the
    // rotation manually
    //GLdouble matrix[16];
    //glPushMatrix();
    //GLUtil::printMatrices();


    /*glTranslatef( x, y, z );
    glRotatef( xRot, 1.0f, 0.0f, 0.0f );
    glRotatef( yRot, 0.0f, 0.0f, 1.0f );
    glRotatef( zRot, 0.0f, 1.0f, 0.0f );
    glGetDoublev( GL_MODELVIEW_MATRIX, matrix );*/

    //glPopMatrix();
    //GLUtil::updateMatrices();
    //GLUtil::printMatrices();
    //for ( int i = 0; i < 16; i++ )
    //    matrix[i] = GLUtil::modelview[i];

    glPushMatrix();
    glLoadIdentity();
    glTranslatef( x, y, z );
    glRotatef( xRot, 1.0f, 0.0f, 0.0f );
    glRotatef( yRot, 0.0f, 0.0f, 1.0f );
    glRotatef( zRot, 0.0f, 1.0f, 0.0f );
    glGetDoublev( GL_MODELVIEW_MATRIX, matrix );
    glPopMatrix();

    float rlat = lat;//-90.0f); //-xRot
    float rlon = lon; //+zRot
    //float rlat = lat-xRot-90.0f;
    //float rlon = lon+zRot;

    rlat = rlat*PI/180.0f;
    rlon = rlon*PI/180.0f;

    float ext = radius * (cos(rlat) * sin(rlon));
    float eyt = radius * (sin(rlat));
    float ezt = radius * (cos(rlat) * cos(rlon));
    //gravUtil::logVerbose( "ex,ey,ez: %f,%f,%f\n", ex, ey, ez );

    /*glBegin( GL_LINE );
    glVertex3f( 0.0f, 0.0f, 0.0f );
    glVertex3f( ex, ey, ez );
    glEnd();*/

    // column major
    ex = (ext*matrix[0]) + (eyt*matrix[4]) + (ezt*matrix[8]) + matrix[12];
    ey = (ext*matrix[1]) + (eyt*matrix[5]) + (ezt*matrix[9]) + matrix[13];
    ez = (ext*matrix[2]) + (eyt*matrix[6]) + (ezt*matrix[10]) + matrix[14];
    //gravUtil::logVerbose( "x,y,z: %f,%f,%f\n", ex, ey, ez );

    // row major
    /*ex = (ex*matrix[0]) + (ey*matrix[1]) + (ez*matrix[2]);// + matrix[3];
    ey = (ex*matrix[4]) + (ey*matrix[5]) + (ez*matrix[5]);// + matrix[7];
    ez = (ex*matrix[8]) + (ey*matrix[9]) + (ez*matrix[10]);// + matrix[11];*/

    //gravUtil::logVerbose( "matrix/modelview: %f, %f\n", matrix[1], GLUtil::modelview[1] );

    //ex = (ex*GLUtil::modelview[0]) + (ey*GLUtil::modelview[4]) + (ez*GLUtil::modelview[8]) + GLUtil::modelview[12];
    //ey = (ex*GLUtil::modelview[1]) + (ey*GLUtil::modelview[5]) + (ez*GLUtil::modelview[9]) + GLUtil::modelview[13];
    //ez = (ex*GLUtil::modelview[2]) + (ey*GLUtil::modelview[3]) + (ez*GLUtil::modelview[10]) + GLUtil::modelview[14];

    //glPopMatrix();
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
