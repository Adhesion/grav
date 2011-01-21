/*
 * @file Point.h
 *
 * Represents a point in 3D space.
 *
 * Created on: Aug 19, 2010
 * @author Andrew Ford
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
     * Returns a new point of adding vector to this point.
     */
    Point operator+( const Vector& v );

    Vector toVector();

private:
    float x, y, z;

};

#endif /* POINT_H_ */
