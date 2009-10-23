/*
 * @file Earth.h
 * Defines a class for rendering a textured Earth sphere, and also provides
 * methods for converting lat/long coordinates to X/Y/Z 3d world space.
 * @author Andrew Ford
 */

#include "Earth.h"
#include "PNGLoader.h"
#include "glutil.h"

#include <GL/gl.h>
#include <GL/glu.h>
#include <cmath>

const float PI = 3.1415926535;

Earth::Earth()
{
    x = 0.0f; y = 0.0f, z = -25.0f;
    radius = 20.0f;
    xRot = -90.0f; yRot = 0.0f; zRot = 0.0f;
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
    glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
    
    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, earthTex );
    
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    gluSphere( sphereQuad, radius, 30, 30 );
    
    glDisable( GL_CULL_FACE );
    glDisable( GL_TEXTURE_2D );
    
    glPopMatrix();
    
    // lat/long testing
    float sx, sy, sz;
    
    glPushMatrix();
    convertLatLong( 0.0f, 0.0f, sx, sy, sz );
    glTranslatef( sx, sy, sz );
    glColor3f( 1.0f, 0.0f, 0.0f );
    gluSphere( sphereQuad, radius/10.0f, 30, 30 );
    glPopMatrix();
    
    glPushMatrix();
    convertLatLong( 43.165556f, -77.611389f, sx, sy, sz );
    glTranslatef( sx, sy, sz );
    glColor3f( 0.0f, 0.0f, 1.0f );
    gluSphere( sphereQuad, radius/10.0f, 30, 30 );
    glPopMatrix();
    
    glPushMatrix();
    convertLatLong( 51.507778f, -0.128056f, sx, sy, sz );
    glTranslatef( sx, sy, sz );
    glColor3f( 0.0f, 1.0f, 0.0f );
    gluSphere( sphereQuad, radius/10.0f, 30, 30 );
    glPopMatrix();
    
    glPushMatrix();
    convertLatLong( 0.0f, -90.0f, sx, sy, sz );
    glTranslatef( sx, sy, sz );
    glColor3f( 0.0f, 1.0f, 1.0f );
    gluSphere( sphereQuad, radius/10.0f, 30, 30 );
    glPopMatrix();
    
    glPushMatrix();
    convertLatLong( 0.0f, 90.0f, sx, sy, sz );
    glTranslatef( sx, sy, sz );
    glColor3f( 1.0f, 1.0f, 0.0f );
    gluSphere( sphereQuad, radius/10.0f, 30, 30 );
    glPopMatrix();
    
    glPushMatrix();
    convertLatLong( testLat, 0.0f, sx, sy, sz );
    glTranslatef( sx, sy, sz );
    glColor3f( 1.0f, 1.0f, 1.0f );
    gluSphere( sphereQuad, radius/10.0f, 30, 30 );
    glPopMatrix();
    
    testLat++;
}

void Earth::convertLatLong( float lat, float lon, float &ex, float &ey,
                            float &ez)
{
    // grab the modelview matrix so we can calculate the result of the
    // rotation manually
    GLdouble matrix[16];
    glPushMatrix();
    glTranslatef( x, y, z );
    glRotatef( xRot, 1.0f, 0.0f, 0.0f );
    glRotatef( yRot, 0.0f, 1.0f, 0.0f );
    glRotatef( zRot, 0.0f, 0.0f, 1.0f );
    GLUtil::updateMatrices();
    for ( int i = 0; i < 16; i++ )
        matrix[i] = GLUtil::modelview[i];
    glPopMatrix();
    
    float rlat = (lat-90.0f); //-xRot
    float rlon = lon; //+zRot
    //float rlat = lat-xRot-90.0f;
    //float rlon = lon+zRot;
    
    rlat = rlat*PI/180.0f;
    rlon = rlon*PI/180.0f;
    
    ex = radius * (cos(rlat) * sin(rlon)) + x;
    ey = radius * (sin(rlat)) + y;
    ez = radius * (cos(rlat) * cos(rlon)) + z;
}

void Earth::rotate( float x, float y, float z )
{
    xRot += x;
    yRot += y;
    zRot += z;
}
