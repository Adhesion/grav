/*
 * @file VideoSource.h
 *
 * Definition of the VideoSource class. A VideoSource is a collection of the
 * video data as well as metadata like the video's position in space (or on the
 * screen canvas) and titles.
 *
 * @author Andrew Ford
 * Copyright (C) 2011 Rochester Institute of Technology
 *
 * This file is part of grav.
 *
 * grav is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * grav is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with grav.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef VIDEOSOURCE_H_
#define VIDEOSOURCE_H_

#include <VPMedia/video/VPMVideoBufferSink.h>
#include <VPMedia/VPMSession.h>
#include <VPMedia/VPMedia_config.h>

#include "RectangleBase.h"

class VideoListener;

class VideoSource : public RectangleBase
{

public:
    VideoSource( VPMSession* _session, VideoListener* l, uint32_t _ssrc,
					VPMVideoBufferSink* vs, float x, float y );
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

    /*
     * Gets the session this video comes from - we need to be able to check this
     * against the session in the delete callback, so we don't delete the wrong
     * one if it happens to have the same ssrc as another stream.
     */
    VPMSession* getSession();

    unsigned int getVideoWidth();
    unsigned int getVideoHeight();

    // overrides the functions from RectangleBase to account for aspect ratio
    float getWidth(); float getHeight();
    float getDestWidth(); float getDestHeight();
    void setWidth( float w ); void setHeight( float h );
    float getOriginalAspect();

    // set/get for mute (controls state of decoder)
    void toggleMute();
    bool isMuted();

    // enable/disable texture push
    void setRendering( bool r );
    bool getRendering();

    // override RectangleBase::show to affect alpha usage for video rendering
    void show( bool s, bool instant );

private:
    // reference to the session that this video comes from - needed for grabbing
    // metadata from RTCP/SDES
    VPMSession* session;
    // reference to listener that made it, to update pixel counts
    VideoListener* listener;

    // synchronization source, from rtp
    uint32_t ssrc;

    // the source of the video data
    VPMVideoBufferSink* videoSink;

    // original dimensions of the video
    unsigned int vwidth, vheight;

    // original aspect ratio of the video
    float aspect;
    float destAspect;

    // override animate to animate aspect
    void animateValues();
    bool aspectAnimating;

    // remake the buffer when the video gets resized
    void resizeBuffer();

    // dimensions rounded up to power of 2
    unsigned int tex_width, tex_height;

    // GL texture identifier
    GLuint texid;
    bool init;

    // whether the texture push is enabled
    bool enableRendering;

    // whether to apply color's alpha to video
    bool useAlpha;
};

#endif /* VIDEOSOURCE_H_ */
