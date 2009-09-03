#ifndef VIDEOSOURCE_H_
#define VIDEOSOURCE_H_

/**
 * @file VideoSource.h
 * Definition of the VideoSource class. A VideoSource is a collection of the
 * video data as well as metadata like the video's position in space (or on the
 * screen canvas) and titles.
 * @author Andrew Ford
 */

#include <VPMedia/video/VPMVideoBufferSink.h>
#include <VPMedia/VPMSession.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>

#include <FTGL/ftgl.h>

#include <string>

class VideoSource
{

public:
    VideoSource( VPMSession* _session, uint32_t _ssrc, VPMVideoBufferSink* vs,
                    float x, float y );
    ~VideoSource();
    
    void draw();
    void scaleNative(); // change the scale of the video to be native size
                        // relative to the screen size
    std::string getMetadata( VPMSession::VPMSession_SDES type );
    float getWidth(); float getHeight();
    // dimensions of the source in world space
    void moveX( float _x ); void moveY( float _y );
    // change the position of the source
    void setScale( float xs, float ys ); // change the scale of the video
    float getX(); float getY(); float getZ();
    float getScaleX(); float getScaleY();
    uint32_t getssrc();
    std::string getName();
    bool isSelected();
    void setSelect( bool select );

private:
    VPMSession* session; // reference to the session that this video comes from
    uint32_t ssrc; // synchronization source, from rtp
    VPMVideoBufferSink* videoSink; // the source of the video data
    unsigned int vwidth, vheight; // original dimensions of the video
    float aspect; // aspect ratio of the video
    std::string name;
    
    unsigned int tex_width, tex_height; // dimensions rounded up to power of 2
    GLuint texid; // GL texture identifier
    FTFont* font;
    
    float x,y,z; // position in world space (center of the video)
    float destX, destY; // x/y destinations for movement
    float angle;
    float scaleX, scaleY;
    float destScaleX, destScaleY;
    bool selected;
    bool animated;

};

#endif /* VIDEOSOURCE_H_ */
