/*
 * Camera.cpp
 *
 * Implementation of the camera. Updates GL matrices, etc.
 *
 * Created on: Jan 18, 2011
 * @author Andrew Ford
 */

#include "Camera.h"
#include "Point.h"
#include "Vector.h"
#include "Earth.h"

Camera::Camera( Point c, Point l )
    : center( c ), lookat( l )
{
    up( 0.0f, 0.0f, 1.0f );
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

void setCenter( float x, float y, float z )
{
    center( x, y, z );
    destCenter( x, y, z );
}

void moveCenter( float x, float y, float z )
{
    destCenter( x, y, z );
    if ( !animated )
        center( x, y, z );
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
}
