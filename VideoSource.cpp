/**
 * @file VideoSource.cpp
 * Implementation of the VideoSource class. See VideoSource.h for details.
 * @author Andrew Ford
 */

#include "VideoSource.h"
#include "GLUtil.h"
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
}

VideoSource::~VideoSource()
{
    delete videoSink;
    
    // gl destructors
    glDeleteTextures( 1, &texid );
}

void VideoSource::draw()
{
    //animateValues();
    // to draw the border/text/common stuff
    RectangleBase::draw();
    
    // set up our position
    glPushMatrix();

    glRotatef(angle, 0.0, 1.0, 0.0);
    glTranslatef(x,y,z);
    
    //glDepthMask( GL_FALSE );
    //glDepthRange (0.0, 0.9);
    //glPolygonOffset( 0.2, 0.8 );
    
    float s = 1.0;
    float t = 1.0;
    // if the texture id hasn't been initialized yet, this must be the
    // first draw call
    init = (texid == 0);

    // allocate the buffer if it's the first time or if it's been resized
    if ( init || vwidth != videoSink->getImageWidth() ||
         vheight != videoSink->getImageHeight() )
    {
        resizeBuffer();
    }
    
    s = (float)vwidth/(float)tex_width;
    //if ( videoSink->getImageFormat() == VIDEO_FORMAT_YUV420 )
    //    t = (float)(3*vheight/2)/(float)tex_height;
    //else
    t = (float)vheight/(float)tex_height;
    
    if ( GLUtil::getInstance()->haveShaders() )
    {
        glUseProgram( GLUtil::getInstance()->getYUV420Program() );
        glUniform1f( GLUtil::getInstance()->getYUV420xOffsetID(), s );
        glUniform1f( GLUtil::getInstance()->getYUV420yOffsetID(), t );
    }
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texid);

    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glPixelStorei( GL_UNPACK_ROW_LENGTH, vwidth );
    
    if ( videoSink->getImageFormat() == VIDEO_FORMAT_RGB24 )
    {
        glTexSubImage2D( GL_TEXTURE_2D,
    	      0,
    	      0,
    	      0,
    	      vwidth,
    	      vheight,
    	      GL_RGB,
    	      GL_UNSIGNED_BYTE,
    	      videoSink->getImageData() );
    }
    
    // if we're doing yuv420, do the texture mapping for all 3 channels so the
    // shader can properly work its magic
    else if ( videoSink->getImageFormat() == VIDEO_FORMAT_YUV420 )
    {
        glTexSubImage2D( GL_TEXTURE_2D,
              0,
              0,
              0,
              vwidth,
              vheight,
              GL_LUMINANCE,
              GL_UNSIGNED_BYTE,
              videoSink->getImageData() );
        
        // now map the U & V to the bottom chunk of the image
        // each is 1/4 of the size of the Y (half width, half height)
        glPixelStorei(GL_UNPACK_ROW_LENGTH, vwidth/2);
        
        glTexSubImage2D( GL_TEXTURE_2D,
              0,
              0,
              vheight,
              vwidth/2,
              vheight/2,
              GL_LUMINANCE,
              GL_UNSIGNED_BYTE,
              (GLubyte*)videoSink->getImageData() + (vwidth*vheight) );
              
        glTexSubImage2D( GL_TEXTURE_2D,
              0,
              vwidth/2,
              vheight,
              vwidth/2,
              vheight/2,
              GL_LUMINANCE,
              GL_UNSIGNED_BYTE,
              (GLubyte*)videoSink->getImageData() + 5*(vwidth*vheight)/4 );
    }
    
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
    
    if ( GLUtil::getInstance()->haveShaders() )
        glUseProgram( 0 );
    
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

void VideoSource::resizeBuffer()
{
    vwidth = videoSink->getImageWidth();
    vheight = videoSink->getImageHeight();
    
    if ( vheight > 0 )
        aspect = (float)vwidth / (float)vheight;
    else
        aspect = 1.33f;
    
    tex_width = GLUtil::getInstance()->pow2(vwidth);
    if ( videoSink->getImageFormat() == VIDEO_FORMAT_YUV420 )
        tex_height = GLUtil::getInstance()->pow2( 3*vheight/2 );
    else
        tex_height = GLUtil::getInstance()->pow2( vheight );
    
    printf( "image size is %ix%i\n", vwidth, vheight );
    printf( "texture size is %ix%i\n", tex_width, tex_height );
    
    // if it's not the first time we're allocating a texture
    // (ie, it's a resize) delete the previous texture
    if ( !init ) glDeleteTextures( 1, &texid );
        glGenTextures(1, &texid);
    
    glBindTexture(GL_TEXTURE_2D, texid);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    unsigned char *buffer = new unsigned char[tex_width * tex_height * 3];
    memset(buffer, 128, tex_width * tex_height * 3);
    glTexImage2D( GL_TEXTURE_2D,
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

void VideoSource::scaleNative()
{
    // get the top left point of the screen in world coords, since that's
    // what the screen coordinates are relative to
    GLdouble topLeftX; GLdouble topLeftY; GLdouble topLeftZ;
    // note: the weird number is because the Z of screen space does actually
    // have an effect - that's what is returned when doing a world->screen
    // conversion for any point at worldZ=0
    GLUtil::getInstance()->screenToWorld( (double)0, (double)0, 0.990991f,
                            &topLeftX, &topLeftY, &topLeftZ );
    
    //printf( "top left of the screen in world coords is %f,%f,%f\n",
    //        topLeftX, topLeftY, topLeftZ );
    
    // now get the world space position of the video dimensions
    GLdouble dimX; GLdouble dimY; GLdouble dimZ;
    GLUtil::getInstance()->screenToWorld( (GLdouble)vwidth, (GLdouble)vheight,
                                                0.990991f,
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
