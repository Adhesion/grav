/*
 * @file Vector.cpp
 *
 * Implementation of direction in 3D space.
 *
 * Created on: Aug 19, 2010
 * @author Andrew Ford
 */

#include "Vector.h"
#include <cmath>

Vector::Vector()
    : x( 0.0f ), y( 0.0f ), z( 0.0f )
{ }

Vector::Vector( float _x, float _y, float _z )
    : x( _x ), y( _y ), z( _z )
{ }

float Vector::getX() const
{
    return x;
}

float Vector::getY() const
{
    return y;
}

float Vector::getZ() const
{
    return z;
}

Vector Vector::crossProduct( const Vector& other )
{
    return Vector( ( y * other.z - z * other.y ),
                   ( z * other.x - x * other.z ),
                   ( x * other.y - y * other.x ) );
}

float Vector::dotProduct( const Vector& other )
{
    return ( ( x * other.x ) + ( y * other.y ) + ( z * other.z ) );
}

void Vector::normalize()
{
    float len = getLength();

    x = x / len;
    y = y / len;
    z = z / len;
}

float Vector::getLength()
{
    return sqrt( ( x * x ) + ( y * y ) + ( z * z ) );
}
