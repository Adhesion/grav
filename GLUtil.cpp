/*
 * @file GLUtil.cpp
 * Implementation of GL/miscellaneous 3D utility functions.
 * @author Andrew Ford
 */

#include "GLUtil.h"

void GLUtil::updateMatrices()
{
    glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
    glGetDoublev( GL_PROJECTION_MATRIX, projection );
    glGetIntegerv( GL_VIEWPORT, viewport );
}

void GLUtil::printMatrices()
{
    updateMatrices();
    
    printf( "printing modelview matrix:\n[" );
    int c = 0;
    for ( int i = 0; i < 16; i++ )
    {
        printf( "%f", modelview[c] );
        if ( i % 4 == 3 )
            printf( "]\n[" );
        else
            printf( " " );
        if ( c >= 16 ) { c=(c+5)%16; }
        else c+=4;
    }
    c = 0;
    
    /*printf( "printing modelview matrix row-major (wrong)\n[" );
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
        if ( c >= 16 ) { c=(c+5)%16; }
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

void GLUtil::worldToScreen( GLdouble x, GLdouble y, GLdouble z,
                                GLdouble* scrX, GLdouble* scrY, GLdouble* scrZ )
{
    updateMatrices();
    
    GLint ret = gluProject( x, y, z, modelview, projection, viewport,
                            scrX, scrY, scrZ );
    if ( ret == 0 )
        printf( "gluproject returned false\n" );
}

void GLUtil::screenToWorld( GLdouble scrX, GLdouble scrY, GLdouble scrZ,
                                GLdouble* x, GLdouble* y, GLdouble* z )
{
    updateMatrices();
    
    gluUnProject( scrX, scrY, scrZ, modelview, projection, viewport,
                    x, y, z );
}