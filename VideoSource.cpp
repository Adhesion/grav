/**
 * @file VideoSource.cpp
 * Implementation of the VideoSource class. See VideoSource.h for details.
 * @author Andrew Ford
 */

#include "VideoSource.h"
#include "GLUtil.h"
#include <cmath>

#include <VPMedia/video/VPMVideoDecoder.h>

VideoSource::VideoSource( VPMSession* _session, uint32_t _ssrc,
                            VPMVideoBufferSink* vs, float _x, float _y ) :
    RectangleBase( _x, _y ), session( _session ), ssrc( _ssrc ), videoSink( vs )
{
    vwidth = videoSink->getImageWidth();
    vheight = videoSink->getImageHeight();
    aspect = (float)vwidth / (float)vheight;
    tex_width = 0; tex_height = 0;
    texid = 0;
    aspect = 1.33f;
    useAlpha = false;
}

VideoSource::~VideoSource()
{
    // note that the buffer sink will be deleted when the decoder for the source
    // is (inside VPMedia), so that's why it isn't deleted here or in
    // videolistener

    // gl destructors
    glDeleteTextures( 1, &texid );
}

void VideoSource::draw()
{
    /*if ( !videoSink->haveNewFrameAvailable() )
        printf( "VideoSource::no new frame available\n" );
    else
        printf( "VideoSource::new frame available!!!\n" );*/
    //printf( "VideoSource::drawing %s at %f,%f, size %f,%f\n", getName().c_str(),
    //        x, y, getWidth(), getHeight() );

    //animateValues();
    // to draw the border/text/common stuff
    RectangleBase::draw();
    
    // set up our position
    glPushMatrix();

    glRotatef( xAngle, 1.0, 0.0, 0.0 );
    glRotatef( yAngle, 0.0, 1.0, 0.0 );
    glRotatef( zAngle, 0.0, 0.0, 1.0 );

    glTranslatef(x,y,z);
    
    //glDepthMask( GL_FALSE );
    //glDepthRange (0.0, 0.9);
    //glPolygonOffset( 0.2, 0.8 );
    
    //float s = 1.0;
    //float t = 1.0;
    // if the texture id hasn't been initialized yet, this must be the
    // first draw call
    init = (texid == 0);

    // allocate the buffer if it's the first time or if it's been resized
    if ( init || vwidth != videoSink->getImageWidth() ||
         vheight != videoSink->getImageHeight() )
    {
        resizeBuffer();
    }
    
    //s = (float)vwidth/(float)tex_width;
    //if ( videoSink->getImageFormat() == VIDEO_FORMAT_YUV420 )
    //    t = (float)(3*vheight/2)/(float)tex_height;
    //else
    //t = (float)vheight/(float)tex_height;
    int s = vwidth;
    int t = vheight;
    
    // X & Y distances from center to edge
    float Xdist = aspect*scaleX/2;
    float Ydist = scaleY/2;

    glBindTexture( GL_TEXTURE_2D, texid );

    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glPixelStorei( GL_UNPACK_ROW_LENGTH, vwidth );

    // only do this texture stuff if rendering is enabled
    if ( enableRendering )
    {
        // only bother doing a texture push if there's a new frame
        if ( videoSink->haveNewFrameAvailable() )
        {
            if ( videoSink->getImageFormat() == VIDEO_FORMAT_RGB24 )
            {
                //printf( "0,0,0: %u, %u, %u\n", ((unsigned char*)(videoSink->getImageData()))[0], ((unsigned char*)(videoSink->getImageData()))[1], ((unsigned char*)(videoSink->getImageData()))[2] );
                //printf( "top left?: %u, %u, %u\n", ((unsigned char*)(videoSink->getImageData()))[(vwidth*(vheight-1)*3)], ((unsigned char*)(videoSink->getImageData()))[(vwidth*(vheight-1)*3)+1], ((unsigned char*)(videoSink->getImageData()))[(vwidth*(vheight-1)*3)+2] );
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

            // if we're doing yuv420, do the texture mapping for all 3 channels
            // so the shader can properly work its magic
            else if ( videoSink->getImageFormat() == VIDEO_FORMAT_YUV420 )
            {
                // experimental single-push method
                glTexSubImage2D( GL_TEXTURE_2D,
                                 0,
                                 0,
                                 0,
                                 vwidth,
                                 3*vheight/2,
                                 GL_LUMINANCE,
                                 GL_UNSIGNED_BYTE,
                                 videoSink->getImageData() );

                // 3 pushes separate
                /*glTexSubImage2D( GL_TEXTURE_2D,
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
                      (GLubyte*)videoSink->getImageData() + 5*(vwidth*vheight)/4 );*/
            }
        }
    }

    // draw video texture, regardless of whether we just pushed something
    // new or not
    if ( GLUtil::getInstance()->useShaders() )
    {
        glUseProgram( GLUtil::getInstance()->getYUV420Program() );
        glUniform1i( GLUtil::getInstance()->getYUV420xOffsetID(), s );
        glUniform1i( GLUtil::getInstance()->getYUV420yOffsetID(), t );
        if ( !selectable )
        {
            glUniform1f( GLUtil::getInstance()->getYUV420alphaID(),
                            borderColor.A );
        }
    }

    // use alpha of border color for video if set
    if ( useAlpha )
    {
        glColor4f( 1.0f, 1.0f, 1.0f, borderColor.A );
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    }
    else
    {
        glColor3f( 1.0f, 1.0f, 1.0f );
    }

    glEnable( GL_TEXTURE_2D );
    glBegin( GL_QUADS );

    // now draw the actual quad that has the texture on it
    // size of the video in world space will be equivalent to getWidth x
    // getHeight, which is the same as (aspect*scaleX) x scaleY
    glTexCoord2i( 0, 0 );
    glVertex3f( -Xdist, -Ydist, 0.0 );

    glTexCoord2i( 0, t );
    glVertex3f( -Xdist, Ydist, 0.0 );

    glTexCoord2i( s, t );
    glVertex3f( Xdist, Ydist, 0.0 );

    glTexCoord2i( s, 0 );
    glVertex3f( Xdist, -Ydist, 0.0 );

    glEnd();

    glDisable( GL_TEXTURE_2D );

    if ( GLUtil::getInstance()->useShaders() )
        glUseProgram( 0 );

    if ( vwidth == 0 || vheight == 0 )
    {
        glPushMatrix();
        glTranslatef( -(getWidth()*0.275f), getHeight()*0.3f, 0.0f );
        float scaleFactor = getTextScale();
        glScalef( scaleFactor, scaleFactor, scaleFactor );
        std::string waitingMessage( "Waiting for video..." );
        font->Render( waitingMessage.c_str() );
        glPopMatrix();
    }

    // draw a basic X in the top-left corner for signifying that rendering is
    // disabled, differentiating between muting and just having the texture
    // push disabled via the color
    if ( !enableRendering )
    {
        float dist = getWidth() * 0.1f;

        glBegin( GL_LINES );
        glLineWidth( 3.0f );

        glColor4f( secondaryColor.R, secondaryColor.G, secondaryColor.B,
                    secondaryColor.A );

        glVertex3f( -Xdist, Ydist - dist, 0.0f );
        glVertex3f( -Xdist + dist, Ydist, 0.0f );

        glVertex3f( -Xdist, Ydist, 0.0f );
        glVertex3f( -Xdist + dist, Ydist - dist, 0.0f );

        glEnd();
    }
    
    // see above
    if ( useAlpha )
    {
        glDisable( GL_BLEND );
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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

    // update text bounds since the width might be different
    updateTextBounds();
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

bool VideoSource::updateName()
{
    bool nameChanged = false;
    std::string sdesName = getMetadata( VPMSession::VPMSESSION_SDES_NAME );
    std::string sdesCname = getMetadata( VPMSession::VPMSESSION_SDES_CNAME );
    
    if ( sdesName != "" && sdesName != name )
    {
        name = sdesName;
        nameChanged = true;
        finalName = true;
        printf( "in updateName, got name: %s\n", name.c_str() );
    }
    if ( sdesCname != "" && sdesCname != altName )
    {
        altName = sdesCname;
        nameChanged = true;
        printf( "in updateName, got cname: %s\n", altName.c_str() );
    }
    
    // if we don't have a proper name yet just use cname
    if ( name == "" && sdesCname != "" )
        name = sdesCname;
    
    // also update the location info
    std::string loc = getMetadata( VPMSession::VPMSESSION_SDES_LOC );
    size_t pos = loc.find( ',' );
    if ( pos != std::string::npos )
    {
        std::string latS = loc.substr( 0, pos );
        std::string lonS = loc.substr( pos+1 );
        lat = strtod( latS.c_str(), NULL );
        lon = strtod( lonS.c_str(), NULL );
    }
    
    if ( nameChanged )
        updateTextBounds();
    return nameChanged;
}

uint32_t VideoSource::getssrc()
{
    return ssrc;
}

std::string VideoSource::getName()
{
    return name;
}

const char* VideoSource::getPayloadDesc()
{
    return videoSink->getVideoDecoder()->getDesc();
}

float VideoSource::getWidth()
{
    return aspect * scaleX;
}

float VideoSource::getHeight()
{
    return scaleY;
}

float VideoSource::getDestWidth()
{
    return aspect * destScaleX;
}

float VideoSource::getDestHeight()
{
    return destScaleY;
}

void VideoSource::setWidth( float w )
{
    setScale( w/aspect, destScaleY * (w/(destScaleX*aspect)) );
}

void VideoSource::setHeight( float h )
{
    setScale( destScaleX * (h/destScaleY), h );
}

void VideoSource::toggleMute()
{
    session->enableSource( ssrc, isMuted() );
    enableRendering = !isMuted();

    if ( isMuted() )
    {
        baseBColor.R = 1.0f; baseBColor.G = 0.1f; baseBColor.B = 0.15f;
        // see resetColor for why we check for selected here
        if ( !selected )
            setColor( baseBColor );
        setSecondaryColor( baseBColor );
    }
    else
    {
        resetColor();
        destSecondaryColor.R = 0.0f; destSecondaryColor.G = 0.0f;
        destSecondaryColor.B = 0.0f; destSecondaryColor.A = 0.0f;
    }
}

bool VideoSource::isMuted()
{
    return !session->isSourceEnabled( ssrc );
}

void VideoSource::setRendering( bool r )
{
    // muting is related to rendering - enablerendering shouldn't be true when
    // it's muted, so stop it from changing when it is muted
    if ( !isMuted() )
    {
        enableRendering = r;

        if ( !enableRendering )
        {
            baseBColor.R = 0.05f; baseBColor.G = 0.1f; baseBColor.B = 1.0f;
            // see resetColor for why we check for selected here
            if ( !selected )
                setColor( baseBColor );
            setSecondaryColor( baseBColor );
        }
        else
        {
            resetColor();
            destSecondaryColor.R = 0.0f; destSecondaryColor.G = 0.0f;
            destSecondaryColor.B = 0.0f; destSecondaryColor.A = 0.0f;
        }
    }
}

void VideoSource::setSelectable( bool s )
{
    RectangleBase::setSelectable( s );
    useAlpha = !selectable;
}
