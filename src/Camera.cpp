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

void Camera::setEarth( Earth* e )
{
    earth = e;
}
