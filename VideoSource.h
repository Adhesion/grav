#ifndef VIDEOSOURCE_H_
#define VIDEOSOURCE_H_

/**
 * @file VideoSource.h
 * Definition of the VideoSource class. A VideoSource is a collection of the
 * video data as well as metadata like the video's position in space (or on the
 * screen canvas) and titles.
 * @author Andrew Ford
 */

#include "RectangleBase.h"

#include <VPMedia/video/VPMVideoBufferSink.h>
#include <VPMedia/VPMSession.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>

class VideoSource : public RectangleBase
{

public:
    VideoSource( VPMSession* _session, uint32_t _ssrc, VPMVideoBufferSink* vs,
                    float x, float y );
    ~VideoSource();
    
    void draw();
    
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
    
    unsigned int tex_width, tex_height; // dimensions rounded up to power of 2
    GLuint texid; // GL texture identifier
    
    int drawCounter; // keeps track of how many times we've drawn

};

#endif /* VIDEOSOURCE_H_ */
