#ifndef VIDEOSOURCE_H_
#define VIDEOSOURCE_H_

/**
 * @file VideoSource.h
 * Definition of the VideoSource class. A VideoSource is a collection of the
 * video data as well as metadata like the video's position in space (or on the
 * screen canvas) and titles.
 * @author Andrew Ford
 */

#ifdef HAVE_GLEW
#include <GL/glew.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <GL/freeglut.h>

#include "RectangleBase.h"

#include <VPMedia/video/VPMVideoBufferSink.h>
#include <VPMedia/VPMSession.h>



class VideoSource : public RectangleBase
{

public:
    VideoSource( VPMSession* _session, uint32_t _ssrc, VPMVideoBufferSink* vs,
                    float x, float y );
    ~VideoSource();
    
    void draw();
    
    /*
     * If we have GLEW, set the GLSL shader program.
     */
    static void setYUV420Program( GLuint p );
    static bool isYUV420shaderInit();
    
    /*
     * Change the scale of the video to be native size
     * relative to the screen size.
     */
    void scaleNative();
    
    /*
     * Retrieve stream metadata (SDES-only) from the VPMedia session.
     */
    std::string getMetadata( VPMSession::VPMSession_SDES type );
    
    /*
     * Updates the overall name label of the source from the SDES metadata:
     * SDES_NAME if it's available, SDES_CNAME if not.
     */
    void updateName();
    
    uint32_t getssrc();
    std::string getName();
    
    // overrides the functions from RectangleBase to account for aspect ratio
    float getWidth(); float getHeight();

private:
    VPMSession* session; // reference to the session that this video comes from
    uint32_t ssrc; // synchronization source, from rtp
    VPMVideoBufferSink* videoSink; // the source of the video data
    unsigned int vwidth, vheight; // original dimensions of the video
    float aspect; // aspect ratio of the video
    
    void resizeBuffer(); // remake the buffer when the video gets resized
    
    unsigned int tex_width, tex_height; // dimensions rounded up to power of 2
    GLuint texid; // GL texture identifier
    bool init;
    
    static GLuint YUV420program;    
    static GLuint YUV420xOffsetID;
    static GLuint YUV420yOffsetID;
    static bool YUV420shaderInit;
    
};

#endif /* VIDEOSOURCE_H_ */
