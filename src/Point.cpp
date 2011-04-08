/*
 * @file Point.cpp
 *
 * Implementation of a point in 3D space.
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
