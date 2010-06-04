#ifndef GLUTIL_H_
#define GLUTIL_H_

/**
 * @file GLUtil.h
 * Miscellaneous functions for dealing with the 3d space, including coordinate
 * conversions, as well as some miscellaneous functions needed for texturing.
 * @author Andrew Ford
 */

#include <GL/glew.h>

// TODO glxew is platform specific, add some #defs to only include it on
// supported platforms (just linux?)
#include <GL/glxew.h>

#include <FTGL/ftgl.h>

#include <iostream>

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
    
    GLuint getYUV420Program();
    GLuint getYUV420xOffsetID();
    GLuint getYUV420yOffsetID();
    
    FTFont* getMainFont();

    /*
     * Returns whether shaders are available to use or not.
     */
    bool useShaders();
    
    void setShaderDisable( bool ds );

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
    
    bool shaders;
    bool disableShaders;
    
    GLuint YUV420Program;
    GLuint YUV420xOffsetID;
    GLuint YUV420yOffsetID;

    FTFont* mainFont;

};

#endif /*GLUTIL_H_*/
