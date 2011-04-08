/*
 * @file Earth.h
 *
 * Defines a class for rendering a textured Earth sphere, and also provides
 * methods for converting lat/long coordinates to X/Y/Z 3d world space.
 *
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

#ifndef EARTH_H_
#define EARTH_H_

#include "GLUtil.h"

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
    float getRadius();

private:
    // texture ID & info
    GLuint earthTex;
    int texWidth, texHeight;

    GLUquadric* sphereQuad;
    GLuint sphereIndex;

    // note, only doing animation for rotation for now
    bool animated;
    // indicator of whether the object is in motion
    bool rotating;
    void animateValues();

    float x, y, z;
    float radius;
    float xRot, yRot, zRot;
    float destXRot, destYRot, destZRot;
    float testLat;

    float moveAmt;

    // keep track of the transformation matrix to use with lat/long conversion
    GLdouble* matrix;

};

#endif /*EARTH_H_*/
