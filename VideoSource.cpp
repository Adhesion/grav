/**
 * @file VideoSource.cpp
 * Implementation of the VideoSource class. See VideoSource.h for details.
 * @author Andrew Ford
 */

#include "VideoSource.h"
#include "glutil.h"
#include <cmath>

#include <FTGL/ftgl.h>

static inline int pow2(int x) {
  int i;
  for (i = 2; i < x; i <<= 1);
  return i;
}

VideoSource::VideoSource( VPMSession* _session, uint32_t _ssrc,
                            VPMVideoBufferSink* vs, float _x, float _y ) :
    session( _session ), ssrc( _ssrc ), videoSink( vs ), x( _x ), y( _y )
{
    vwidth = videoSink->getImageWidth();
    vheight = videoSink->getImageHeight();
    aspect = (float)vwidth / (float)vheight;
    z = 0.0f; angle = 0.0f;
    tex_width = 0; tex_height = 0;
    texid = 0;
    scaleX = 5.0f; scaleY = 5.0f;
    destX = x; destY = y;
    destScaleX = scaleX; destScaleY = scaleY;
    selected = false;
    animated = true;
}

VideoSource::~VideoSource()
{
    // the videoSink comes from the session, and ours is just a reference to its
    // so we can let it delete it when it itself needs to
    
    // gl destructors
    glDeleteTextures( 1, &texid );
}

void VideoSource::draw()
{
    // movement animation
    x += (destX-x)/10.0f;
    y += (destY-y)/10.0f;
    scaleX += (destScaleX-scaleX)/10.0f;
    scaleY += (destScaleY-scaleY)/10.0f;
    // snap to the destination, since we'll never actually get there via
    // the above lines due to roundoff errors
    if ( fabs(destX-x) < 0.01f ) x = destX;
    if ( fabs(destY-y) < 0.01f ) y = destY;
    if ( fabs(destScaleX-scaleX) < 0.01f ) scaleX = destScaleX;
    if ( fabs(destScaleY-scaleY) < 0.01f ) scaleY = destScaleY;
    
    // set up our position
    glPushMatrix();

    glRotatef(angle, 0.0, 1.0, 0.0);
    glTranslatef(x,y,z);
    
    // draw the border first
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
    
    // if selected the border should be yellow
    if ( selected ) glColor4f ( 1.0f, 1.0f, 0.0f, 0.8f );
    else glColor4f( 1.0f, 1.0f, 1.0f, 0.5f );
    
    glVertex3f(-getWidth()/2.0-0.3, -getHeight()/2.0-0.3, 0.0);
    glVertex3f(-getWidth()/2.0-0.3, getHeight()/2.0+0.3, 0.0);
    glVertex3f(getWidth()/2.0+0.3, getHeight()/2.0+0.3, 0.0);
    glVertex3f(getWidth()/2.0+0.3, -getHeight()/2.0-0.3, 0.0);
    
    glEnd();
    glDisable(GL_BLEND);
    
    float s = 1.0;
    float t = 1.0;
    // if the texture id hasn't been initialized yet, this must be the
    // first draw call
    bool init = (texid == 0);

    // allocate the buffer iff it's the first time or if it's been resized
    if ( init || vwidth != videoSink->getImageWidth() ||
        vheight != videoSink->getImageHeight() ) {
      vwidth = videoSink->getImageWidth();
      vheight = videoSink->getImageHeight();
      aspect = (float)vwidth / (float)vheight;
      tex_width = pow2(vwidth);
      tex_height = pow2(vheight);
      printf( "image size is %ix%i\n", vwidth, 
              vheight );
      printf( "texture size is %ix%i\n", tex_width, 
              tex_height );

      // if it's not the first time we're allocating a texture
      // (ie, it's a resize) delete the previous texture
      if ( !init ) glDeleteTextures( 1, &texid );
      glGenTextures(1, &texid);

      glBindTexture(GL_TEXTURE_2D, texid);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

      unsigned char *buffer = new unsigned char[tex_width * tex_height];
      memset(buffer, 128, tex_width * tex_height);
      glTexImage2D(GL_TEXTURE_2D, 
	     0, 
	     GL_RGB, 
	     tex_width, 
	     tex_height, 
	     0,
	     GL_LUMINANCE,
	     GL_UNSIGNED_BYTE,
	     buffer);
      delete [] buffer;
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texid);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 
	    videoSink->getImageWidth());
    
    glTexSubImage2D(GL_TEXTURE_2D,
	      0,
	      0, 
	      0,
	      vwidth,
	      vheight,
	      GL_RGB,
	      GL_UNSIGNED_BYTE,
	      videoSink->getImageData());

    s = (float)vwidth/(float)tex_width;
    t = (float)vheight/(float)tex_height;

    glBegin(GL_QUADS);
    glColor3f( 1.0f, 1.0f, 1.0f );
    
    // size of the video in world space will be equivalent to getWidth x
    // getHeight, which is the same as (aspect*scaleX) x scaleY
    glTexCoord2f(0.0, 0.0);
    glVertex3f(-getWidth()/2, -getHeight()/2, 0.0);
    
    glTexCoord2f(0.0, t);
    glVertex3f(-getWidth()/2, getHeight()/2, 0.0);

    glTexCoord2f(s, t);
    glVertex3f(getWidth()/2, getHeight()/2, 0.0);

    glTexCoord2f(s, 0.0);
    glVertex3f(getWidth()/2, -getHeight()/2, 0.0);

    glEnd();
    
    glDisable(GL_TEXTURE_2D);    
    //glTranslatef(x, -y+getHeight()/2 - 0.2f, z);
    //glScalef( 5.0f, 5.0f, 5.0f );
    glColor3f( 1.0f, 1.0f, 1.0f );
    std::string test = std::string("test");
    //printf( "starting to write string\n" );
    //const char* c = test.c_str();
    //const unsigned char* uc = (const unsigned char*)test.c_str();
    name = getMetadata(VPMSession::VPMSESSION_SDES_NAME);
    const char* nc = name.c_str();
    //char* c = "test";
    
    float spacing = 0.20f;
    //float i = -name.length()*spacing/2.0f;
    float i = -getWidth();
    for (; *nc != '\0'; nc++)
    {
        glRasterPos2f( i*spacing, getHeight()/2.0f+0.2f );
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_10,*nc);
        i++;
    }
    //glutStrokeString(GLUT_STROKE_MONO_ROMAN, uc);
    
    glPopMatrix();
}

void VideoSource::scaleNative()
{
    // get the top left point of the screen in world coords, since that's
    // what the screen coordinates are relative to
    GLdouble topLeftX; GLdouble topLeftY; GLdouble topLeftZ;
    // note: the weird number is because the Z of screen space does actually
    // have an effect - that's what is returned when doing a world->screen
    // conversion for any point at worldZ=0
    screenToWorld( (double)0, (double)0, 0.990991f, &topLeftX, &topLeftY, &topLeftZ );
    
    //printf( "top left of the screen in world coords is %f,%f,%f\n",
    //        topLeftX, topLeftY, topLeftZ );
    
    // now get the world space position of the video dimensions
    GLdouble dimX; GLdouble dimY; GLdouble dimZ;
    screenToWorld( (GLdouble)vwidth, (GLdouble)vheight, 0.990991f,
                    &dimX, &dimY, &dimZ );
    
    //printf( "video dims in world coords are %f,%f,%f\n",
    //        dimX, dimY, dimZ );
    
    // the difference between top-left and where the video would be is
    // equal to the size of the video dimensions in world coords
    setScale( (dimX-topLeftX)/aspect, dimY-topLeftY );
}

std::string VideoSource::getMetadata( VPMSession::VPMSession_SDES type )
{
    char buffer[256];
    uint32_t bufferLen = sizeof( buffer );
    std::string temp = std::string();

    if ( session->getRemoteSDES( ssrc, type, buffer, bufferLen ) )
        temp = std::string( buffer );

    return temp;
}

float VideoSource::getWidth()
{
    return aspect * scaleX;
}

float VideoSource::getHeight()
{
    return scaleY;
}

void VideoSource::moveX( float _x )
{
    destX = _x;
    if ( !animated ) x = _x;
}

void VideoSource::moveY( float _y )
{
    destY = _y;
    if ( !animated ) y = _y;
}

void VideoSource::setScale( float xs, float ys )
{
    destScaleX = xs; destScaleY = ys;
    if ( !animated) { scaleX = xs; scaleY = ys; }
}

float VideoSource::getX()
{
    return x;
}

float VideoSource::getY()
{
    return y;
}

float VideoSource::getZ()
{
    return z;
}

float VideoSource::getScaleX()
{
    return scaleX;
}

float VideoSource::getScaleY()
{
    return scaleY;
}

uint32_t VideoSource::getssrc()
{
    return ssrc;
}

std::string VideoSource::getName()
{
	return name;
}

bool VideoSource::isSelected()
{
    return selected;
}

void VideoSource::setSelect( bool select )
{
    selected = select;
}
