/*
 * @file Vector.h
 *
 * Represents a direction in 3D space.
 *
 * Created on: Aug 19, 2010
 * @author Andrew Ford
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
