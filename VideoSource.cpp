/**
 * @file VideoSource.cpp
 * Implementation of the VideoSource class. See VideoSource.h for details.
 * @author Andrew Ford
 */

#include "VideoSource.h"
#include "glutil.h"
#include <cmath>

VideoSource::VideoSource( VPMSession* _session, uint32_t _ssrc,
                            VPMVideoBufferSink* vs, float _x, float _y ) :
    session( _session ), ssrc( _ssrc ), videoSink( vs ), RectangleBase(_x,_y)
{
    vwidth = videoSink->getImageWidth();
    vheight = videoSink->getImageHeight();
    aspect = (float)vwidth / (float)vheight;
    tex_width = 0; tex_height = 0;
    texid = 0;
    drawCounter = 0;
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
    animateValues();
    
    // set up our position
    glPushMatrix();

    glRotatef(angle, 0.0, 1.0, 0.0);
    glTranslatef(x,y,z);
    
    RectangleBase::draw();
    
    float s = 1.0;
    float t = 1.0;
    // if the texture id hasn't been initialized yet, this must be the
    // first draw call
    bool init = (texid == 0);

    // allocate the buffer if it's the first time or if it's been resized
    if ( init || vwidth != videoSink->getImageWidth() ||
        vheight != videoSink->getImageHeight() ) {
      vwidth = videoSink->getImageWidth();
      vheight = videoSink->getImageHeight();
      if ( vheight > 0 )
        aspect = (float)vwidth / (float)vheight;
      else
        aspect = 1.33f;
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
    
    // X & Y distances from center to edge
    float Xdist = getWidth()/2;
    float Ydist = getHeight()/2;

    glBegin(GL_QUADS);
    glColor3f( 1.0f, 1.0f, 1.0f );
    
    // size of the video in world space will be equivalent to getWidth x
    // getHeight, which is the same as (aspect*scaleX) x scaleY
    glTexCoord2f(0.0, 0.0);
    glVertex3f(-Xdist, -Ydist, 0.0);
    
    glTexCoord2f(0.0, t);
    glVertex3f(-Xdist, Ydist, 0.0);

    glTexCoord2f(s, t);
    glVertex3f(Xdist, Ydist, 0.0);

    glTexCoord2f(s, 0.0);
    glVertex3f(Xdist, -Ydist, 0.0);

    glEnd();
    
    glDisable(GL_TEXTURE_2D);
    
    if ( vwidth == 0 || vheight == 0 )
    {      
        glPushMatrix();
        glTranslatef( -2.0f, 1.5f, 0.0f );
        float scaleFactor = 0.001f * scaleX;
        glScalef( scaleFactor, scaleFactor, scaleFactor );
        std::string waitingMessage( "Waiting for video..." );
        font->Render( waitingMessage.c_str() );
        glPopMatrix();
    }
    
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

void VideoSource::updateName()
{
    std::string tempName = getMetadata( VPMSession::VPMSESSION_SDES_NAME );
    if ( tempName == "" )
    {
        name = getMetadata( VPMSession::VPMSESSION_SDES_CNAME );
        //printf( "in updateName, got cname: %s\n", name.c_str() );
    }
    else
    {
        name = tempName;
        finalName = true;
        //printf( "in updateName, got name: %s\n", name.c_str() );
    }
}

float VideoSource::getWidth()
{
    return aspect * scaleX;
}

float VideoSource::getHeight()
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
