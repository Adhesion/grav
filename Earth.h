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
    void convertLatLong( float lat, float lon, float &ex, float &ey,
                        float &ez );
    void rotate( float x, float y, float z );
    
private:
    GLuint earthTex;
    GLUquadric* sphereQuad;
    float x, y, z;
    float radius;
    float xRot, yRot, zRot;
    int texWidth, texHeight;
    float testLat;
    
};

#endif /*EARTH_H_*/
