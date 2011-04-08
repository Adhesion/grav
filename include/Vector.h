/*
 * @file Vector.h
 *
 * Represents a direction in 3D space.
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

#ifndef VECTOR_H_
#define VECTOR_H_

class Vector
{

public:
    Vector();
    Vector( float _x, float _y, float _z );

    float getX() const; float getY() const; float getZ() const;

    Vector crossProduct( const Vector& other );
    float dotProduct( const Vector& other );

    void normalize();
    float getLength();

    Vector operator/( const float& factor );

private:
    float x, y, z;

};

#endif /* VECTOR_H_ */
