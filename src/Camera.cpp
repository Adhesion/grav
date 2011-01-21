/*
 * Camera.cpp
 *
 * Implementation of the camera. Updates GL matrices, etc.
 *
 * Created on: Jan 18, 2011
 * @author Andrew Ford
 */

#include "Camera.h"
#include "Earth.h"

Camera::Camera( Point c, Point l )
    : center( c ), lookat( l )
{
    up = Vector( 0.0f, 1.0f, 0.0f );
    destCenter = center;
    destLookat = lookat;
    earth = NULL;
    animated = true;
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

Point Camera::getLookat()
{
    return lookat;
}

Vector Camera::getLookatDir()
{
    return lookat - center;
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
}

void Camera::moveCenter( Point p )
{
    destCenter = p;
    if ( !animated )
        center = p;
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
}

void Camera::moveLookat( Point p )
{
    destLookat = p;
    if ( !animated )
        lookat = p;
}

void Camera::setEarth( Earth* e )
{
    earth = e;
}

void Camera::animateValues()
{
    center = center + ( ( destCenter - center ) / 5.0f );
    lookat = lookat + ( ( destLookat - lookat ) / 5.0f );
    // not doing anim for up vector yet, might have to be different?

    if ( center.findDistance( destCenter ) < 0.01f )
        center = destCenter;
    if ( lookat.findDistance( destLookat ) < 0.01f )
        lookat = destLookat;
}
