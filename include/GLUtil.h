/*
 * @file GLUtil.h
 *
 * Miscellaneous functions for dealing with the 3d space, including coordinate
 * conversions, as well as some miscellaneous functions needed for texturing.
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

#ifndef GLUTIL_H_
#define GLUTIL_H_

// TODO glxew is platform specific, add some #defs to only include it on
// supported platforms (just linux?)
// otherwise include straight glew.h (which glxew.h includes itself)
#include <GL/glxew.h>

#include <FTGL/ftgl.h>

#include <iostream>

#include "Point.h"

class RectangleBase;

class GLUtil
{

public:
    static GLUtil* getInstance();

    /*
     * Call glew init function, check for shaders, and load them if we can.
     */
    bool initGL();
    void cleanupGL();

    // get the matrices that define the camera transforms so we can use those
    // to convert our coordinates
    void updateMatrices();

    inline int pow2( int x )
    {
        int i;
        for (i = 2; i < x; i <<= 1);
        return i;
    }

    /**
     * Prints out the current modelview, projection and viewport matrices.
     */
    void printMatrices();

    /**
     * Converts world coordinates to screen coordinates (ie, pixels)
     *
     * @param   x, y, z             x,y,z in world space
     * @param   scrX, scrY, scrZ    screen coordinate x,y,z in screen space
     *                              also acts as the return vals since they're
     *                              passed as pointers
     */
    void worldToScreen( GLdouble x, GLdouble y, GLdouble z,
                        GLdouble* scrX, GLdouble* scrY, GLdouble* scrZ );
    void worldToScreen( Point worldPoint, Point& screenPoint );

    /**
     * Converts screen coordinates (ie, pixels) to world space
     *
     * @param   scrX, scrY, scrZ    screen coordinates in pixels
     * @param   x, y, z             world coordinates
     *                              also acts as the return vals since they're
     *                              passed as pointers
     */
    void screenToWorld( GLdouble scrX, GLdouble scrY, GLdouble scrZ,
                        GLdouble* x, GLdouble* y, GLdouble* z );
    void screenToWorld( Point screenPoint, Point& worldPoint );

    /*
     * Take screen x,y, project out from camera point and find intersect point
     * with rect.
     */
    bool screenToRectIntersect( GLdouble x, GLdouble y, RectangleBase rect,
                                    Point& intersect );

    /**
     * Uses GLEW to load a shader (vert and frag) from the string and returns a
     * reference to the program.
     */
    GLuint loadShaders( const char* location );

    GLuint getYUV420Program();
    GLuint getYUV420xOffsetID();
    GLuint getYUV420yOffsetID();
    GLuint getYUV420alphaID();

    FTFont* getMainFont();

    /*
     * Returns whether shaders are available to use or not.
     * Note shader enable needs to be set before initGL is called for the
     * shader check to occur.
     */
    bool areShadersAvailable();

    void setShaderEnable( bool es );

    void setBufferFontUsage( bool buf );

protected:
    GLUtil();
    ~GLUtil();

private:
    static GLUtil* instance;

    // pointers to the GL matrices
    GLdouble modelview[16];
    GLdouble projection[16];
    GLint viewport[4];

    const GLchar* frag420;
    const GLchar* vert420;

    bool shadersAvailable;
    bool enableShaders;

    GLuint YUV420Program;
    GLuint YUV420xOffsetID;
    GLuint YUV420yOffsetID;
    GLuint YUV420alphaID;

    FTFont* mainFont;
    // switch to change to use buffer font - texture font is default
    bool useBufferFont;

};

#endif /*GLUTIL_H_*/
