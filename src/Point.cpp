/*
 * @file Point.cpp
 *
 * Implementation of a point in 3D space.
 *
 * Created on: Aug 19, 2010
 * @author Andrew Ford
 */

#include "Point.h"
#include "Vector.h"
#include <cmath>

Point::Point()
    : x( 0.0f ), y( 0.0f ), z( 0.0f )
{ }

Point::Point( float _x, float _y, float _z )
    : x( _x ), y( _y ), z( _z )
{ }

float Point::getX() const
{
    return x;
}

float Point::getY() const
{
    return y;
}

float Point::getZ() const
{
    return z;
}

void Point::setX( float _x )
{
    x = _x;
}

void Point::setY( float _y )
{
    y = _y;
}

void Point::setZ( float _z )
{
    z = _z;
}

float Point::findDistance( const Point& other )
{
    return sqrt( pow( x - other.x, 2 ) + pow( y - other.y, 2 ) +
                    pow( z - other.z, 2 ));
}

Vector Point::operator-( const Point& other )
{
    return Vector( x - other.x, y - other.y, z - other.z );
}

Point Point::operator+( const Vector& v )
{
    return Point( x + v.getX(), y + v.getY(), z + v.getZ() );
}

Vector Point::toVector()
{
    return Vector( x, y, z );
}
