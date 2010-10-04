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
     * Also set the alternate name to CNAME if it's available.
     */
    bool updateName();
    
    uint32_t getssrc();
    std::string getName();
    
    const char* getPayloadDesc();

    unsigned int getVideoWidth();
    unsigned int getVideoHeight();

    // overrides the functions from RectangleBase to account for aspect ratio
    float getWidth(); float getHeight();
    float getDestWidth(); float getDestHeight();
    void setWidth( float w ); void setHeight( float h );

    // set/get for mute (controls state of decoder)
    void toggleMute();
    bool isMuted();

    // override RectangleBase::setRendering to account for muting
    void setRendering( bool r );
    // override RectangleBase::setSelectable to affect alpha usage
    void setSelectable( bool s );

private:
    // reference to the session that this video comes from - needed for grabbing
    // metadata from RTCP/SDES
    VPMSession* session;

    // synchronization source, from rtp
    uint32_t ssrc;

    // the source of the video data
    VPMVideoBufferSink* videoSink;

    // original dimensions of the video
    unsigned int vwidth, vheight;

    // aspect ratio of the video
    float aspect;

    // remake the buffer when the video gets resized
    void resizeBuffer();

    // dimensions rounded up to power of 2
    unsigned int tex_width, tex_height;

    // GL texture identifier
    GLuint texid;
    bool init;

    // whether to apply color's alpha to video
    bool useAlpha;
};

#endif /* VIDEOSOURCE_H_ */
