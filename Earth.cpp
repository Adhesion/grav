/*
 * @file Earth.h
 * Defines a class for rendering a textured Earth sphere, and also provides
 * methods for converting lat/long coordinates to X/Y/Z 3d world space.
 * @author Andrew Ford
 */

#include "Earth.h"
#include "PNGLoader.h"

#include <GL/gl.h>
#include <GL/glu.h>

Earth::Earth()
{
    x = 0.0f; y = 0.0f, z = -25.0f;
    xRot = 0.0f; yRot = 0.0f; zRot = 0.0f;
    earthTex = PNGLoader::loadPNG( "earth.png", texWidth, texHeight );
    sphereQuad = gluNewQuadric();
    gluQuadricTexture( sphereQuad, GL_TRUE );
}

Earth::~Earth()
{
    glDeleteTextures( 1, &earthTex );
    gluDeleteQuadric( sphereQuad );
}

void Earth::draw()
{
    glPushMatrix();
    
    glTranslatef( x, y, z );
    glRotatef( xRot, 1.0f, 0.0f, 0.0f );
    glRotatef( yRot, 0.0f, 1.0f, 0.0f );
    glRotatef( zRot, 0.0f, 0.0f, 1.0f );
    
    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, earthTex );
    
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    gluSphere( sphereQuad, 20.0f, 30, 30 );
    
    glDisable( GL_CULL_FACE );
    glDisable( GL_TEXTURE_2D );
    
    glPopMatrix();
}

void Earth::rotate( float x, float y, float z )
{
    xRot += x;
    yRot += y;
    zRot += z;
}
