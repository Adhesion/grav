/*
 * @file Point.h
 *
 * Represents a point in 3D space.
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

#ifndef POINT_H_
#define POINT_H_

class Vector;

class Point
{

public:
    Point();
    Point( float _x, float _y, float _z );

    float getX() const; float getY() const; float getZ() const;
    void setX( float _x ); void setY( float _y ); void setZ( float _z );
    float findDistance( const Point& other );

    /*
     * Returns the vector distance between two points.
     */
    Vector operator-( const Point& other );
    /*
     * Returns a new point: result of adding vector v to this point.
     */
    Point operator+( const Vector& v );

    Vector toVector();

private:
    float x, y, z;

};

#endif /* POINT_H_ */
