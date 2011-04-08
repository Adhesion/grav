/*
 * @file Vector.cpp
 *
 * Implementation of direction in 3D space.
 *
 * Created on: Aug 19, 2010
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

Vector Vector::operator/( const float& factor )
{
    if ( factor < 0.0001f && factor > -0.0001f )
        return Vector( x, y, z );
    return Vector( x / factor, y / factor, z / factor );
}
