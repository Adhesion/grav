#ifndef EARTH_H_
#define EARTH_H_

/*
 * @file Earth.h
 * Defines a class for rendering a textured Earth sphere, and also provides
 * methods for converting lat/long coordinates to X/Y/Z 3d world space.
 * @author Andrew Ford
 */

#include "GLUtil.h"
#include "Vector.h"

class Point;

class Earth
{

public:
    Earth();
    ~Earth();
    void draw();
    void convertLatLong( float lat, float lon, float &ex, float &ey,
                        float &ez );
    void rotate( float x, float y, float z );

    float getX(); float getY(); float getZ();
    Point getPoint();
    float getRadius();

    void setXRotationAxis( Vector v );

private:
    // texture ID & info
    GLuint earthTex;
    int texWidth, texHeight;

    GLUquadric* sphereQuad;
    GLuint sphereIndex;

    float x, y, z;
    float radius;
    float xRot, yRot, zRot;
    float testLat;

    Vector xRotationAxis;

    float moveAmt;

    // keep track of the transformation matrix to use with lat/long conversion
    GLdouble* matrix;

};

#endif /*EARTH_H_*/
