/*
 * @file Camera.cpp
 *
 * Implementation of the camera. Updates GL matrices, etc.
 *
 * Created on: Jan 18, 2011
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

#include "Camera.h"
#include "Earth.h"

Camera::Camera( Point c, Point l )
    : center( c ), lookat( l )
{
    earth = NULL;
    animated = true;
    centerMoving = false;
    lookatMoving = false;

    up = Vector( 0.0f, 1.0f, 0.0f );
    destCenter = center;
    destLookat = lookat;
}

void Camera::doGLLookat()
{
    glLoadIdentity();
    gluLookAt( center.getX(), center.getY(), center.getZ(),
                lookat.getX(), lookat.getY(), lookat.getZ(),
                up.getX(), up.getY(), up.getZ() );
}

Point Camera::getCenter()
{
    return center;
}

Point Camera::getDestCenter()
{
    return destCenter;
}

Point Camera::getLookat()
{
    return lookat;
}

Point Camera::getDestLookat()
{
    return destLookat;
}

Vector Camera::getLookatDir()
{
    return lookat - center;
}

Vector Camera::getDestLookatDir()
{
    return destLookat - destCenter;
}

void Camera::setCenter( float x, float y, float z )
{
    center = Point( x, y, z );
    destCenter = Point( x, y, z );
}

void Camera::setCenter( Point p )
{
    center = p;
    destCenter = p;
}

void Camera::moveCenter( float x, float y, float z )
{
    destCenter = Point( x, y, z );
    if ( !animated )
        center = Point( x, y, z );
    else
        centerMoving = true;
}

void Camera::moveCenter( Point p )
{
    destCenter = p;
    if ( !animated )
        center = p;
    else
        centerMoving = true;
}

void Camera::setLookat( float x, float y, float z )
{
    lookat = Point( x, y, z );
    destLookat = Point( x, y, z );
}

void Camera::setLookat( Point p )
{
    lookat = p;
    destLookat = p;
}

void Camera::moveLookat( float x, float y, float z )
{
    destLookat = Point( x, y, z );
    if ( !animated )
        lookat = Point( x, y, z );
    else
        lookatMoving = true;
}

void Camera::moveLookat( Point p )
{
    destLookat = p;
    if ( !animated )
        lookat = p;
    else
        lookatMoving = true;
}

void Camera::setEarth( Earth* e )
{
    earth = e;
}

void Camera::animateValues()
{
    if ( centerMoving )
    {
        center = center + ( ( destCenter - center ) / 5.0f );

        if ( center.findDistance( destCenter ) < 0.01f )
        {
            center = destCenter;
            centerMoving = false;
        }
    }

    if ( lookatMoving )
    {
        lookat = lookat + ( ( destLookat - lookat ) / 5.0f );

        if ( lookat.findDistance( destLookat ) < 0.01f )
        {
            lookat = destLookat;
            lookatMoving = false;
        }
    }

    // not doing anim for up vector yet, might have to be different?
}
