#ifndef GLUTIL_H_
#define GLUTIL_H_

/**
 * @file glutil.h
 * Miscellaneous functions for dealing with the 3d space, including coordinate
 * conversions.
 * @author Andrew Ford
 */
 
#include <GL/glu.h>

// pointers to the GL matrices in question
static GLdouble modelview[16];
static GLdouble projection[16];
static GLint viewport[4];

static inline int pow2(int x) {
  int i;
  for (i = 2; i < x; i <<= 1);
  return i;
}

/**
 * Prints out the current modelview, projection and viewport matrices.
 */
static void printMatrices()
{
    glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
    glGetDoublev( GL_PROJECTION_MATRIX, projection );
    glGetIntegerv( GL_VIEWPORT, viewport );
    
    printf( "printing modelview matrix:\n[" );
    int c = 0;
    for ( int i = 0; i < 16; i++ )
    {
        printf( "%f", modelview[c] );
        if ( i % 4 == 3 )
            printf( "]\n[" );
        else
            printf( " " );
        if ( c >= 16 ) { c=c%16;c++; }
        else c+=4;
    }
    c = 0;
    
    /*printf( "printing modelview matrix wrong\n[" );
    for ( int i = 0; i < 16; i++ )
    {
        printf( "%f", modelview[i] );
        if ( i % 4 == 3 )
            printf( "]\n[" );
        else
            printf( " " );
    }
    c=0;*/
    
    printf( "printing projection matrix\n[" );
    for ( int i = 0; i < 16; i++ )
    {
        printf( "%f", projection[c%16] );
        if ( i % 4 == 3 )
            printf( "]\n[" );
        else
            printf( " " );
        c += 4;
        if ( c >= 16 ) { c=c%16;c++; }
        else c+=4;
    }
    c = 0;
    
    printf( "printing viewport matrix\n[" );
    for ( int i = 0; i < 4; i++ )
    {
        printf( "%i", viewport[i] );
        if ( i % 2 == 1 ) printf( "]\n" );
        else if ( i == 1 ) printf( "[" );
        else printf( " " );
        if ( c >= 4 ) { c=c%4;c++; }
        else c+=2;
    }
}
 
/**
 * Converts world coordinates to screen coordinates (ie, pixels)
 * 
 * @param   x, y, z             x,y,z in world space
 * @param   scrX, scrY, scrZ    screen coordinate x,y,z in screen space
 *                              also acts as the return vals since they're
 *                              passed as pointers
 */
static void worldToScreen( GLdouble x, GLdouble y, GLdouble z,
                            GLdouble* scrX, GLdouble* scrY, GLdouble* scrZ )
{
    // get the matrices that define the camera transforms so we can use those
    // to convert our coordinates
    glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
    glGetDoublev( GL_PROJECTION_MATRIX, projection );
    glGetIntegerv( GL_VIEWPORT, viewport );
    
    GLint ret = gluProject( x, y, z, modelview, projection, viewport,
                            scrX, scrY, scrZ );
    if ( ret == 0 )
        printf( "gluproject returned false\n" );
}

/**
 * Converts screen coordinates (ie, pixels) to world space
 * 
 * @param   scrX, scrY, scrZ    screen coordinates in pixels
 * @param   x, y, z             world coordinates
 *                              also acts as the return vals since they're
 *                              passed as pointers
 */
static void screenToWorld( GLdouble scrX, GLdouble scrY, GLdouble scrZ,
                            GLdouble* x, GLdouble* y, GLdouble* z )
{
    // see above
    glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
    glGetDoublev( GL_PROJECTION_MATRIX, projection );
    glGetIntegerv( GL_VIEWPORT, viewport );
    
    gluUnProject( scrX, scrY, scrZ, modelview, projection, viewport,
                    x, y, z );
}

#endif /*GLUTIL_H_*/
