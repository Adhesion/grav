#ifndef GLUTIL_H_
#define GLUTIL_H_

/**
 * @file GLUtil.h
 * Miscellaneous functions for dealing with the 3d space, including coordinate
 * conversions, as well as some miscellaneous functions needed for texturing.
 * @author Andrew Ford
 */

#ifdef HAVE_GLEW
#include <GL/glew.h>
#else
#include <GL/glu.h>
#endif

#include <iostream>

namespace GLUtil
{

    // pointers to the GL matrices in question
    static GLdouble modelview[16];
    static GLdouble projection[16];
    static GLint viewport[4];
    
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
    
    /**
     * Uses GLEW to load a shader (.vert and .frag) from files and returns a
     * reference to the program.
     */
    GLuint loadShaders( const char* location );

}

#endif /*GLUTIL_H_*/
