/*
 * @file VideoListener.h
 *
 * Definition of the VideoListener class, which defines behavior on receiving
 * incoming video streams, deleting video streams, and receiving RTCP data.
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

#ifndef VIDEOLISTENER_H_
#define VIDEOLISTENER_H_

#include <VPMedia/VPMSession.h>

#include <sys/time.h>

class gravManager;
class VPMVideoSink;
class GLCanvas;
class wxStopWatch;

//static void newFrameCallbackTest( VPMVideoSink* sink, int buffer_idx,
//                                void* user_data );

class VideoListener : public VPMSessionListener
{

public:
    VideoListener( gravManager* g );
    virtual void vpmsession_source_created( VPMSession &session,
                                          uint32_t ssrc,
                                          uint32_t pt,
                                          VPMPayload type,
                                          VPMPayloadDecoder *decoder );
    virtual void vpmsession_source_deleted( VPMSession &session,
                                          uint32_t ssrc,
                                          const char *reason );
    virtual void vpmsession_source_description( VPMSession &session,
                                              uint32_t ssrc );
    virtual void vpmsession_source_app(VPMSession &session,
                                     uint32_t ssrc,
                                     const char *app,
                                     const char *data,
                                     uint32_t data_len);

    void setTimer( wxStopWatch* t );

    int getSourceCount();
    long getPixelCount();
    void updatePixelCount( long mod );

private:
    gravManager* grav;
    wxStopWatch* timer;

    // initial starting points for videos
    float x;
    float y;
    float initialX;
    float initialY;

    int sourceCount;
    long pixelCount;

};

#endif /*VIDEOLISTENER_H_*/
