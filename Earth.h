#ifndef EARTH_H_
#define EARTH_H_

/*
 * @file Earth.h
 * Defines a class for rendering a textured Earth sphere, and also provides
 * methods for converting lat/long coordinates to X/Y/Z 3d world space.
 * @author Andrew Ford
 */

#include <GL/glu.h>

class Earth
{
    
public:
    Earth();
    ~Earth();
    void draw();
    void convertLatLong( float lat, float lon, float &x, float &y, float &z );
    void rotate( float x, float y, float z );
    
private:
    GLuint earthTex;
    GLUquadric* sphereQuad;
    float x, y, z;
    float xRot, yRot, zRot;
    int texWidth, texHeight;
    
};

#endif /*EARTH_H_*/
