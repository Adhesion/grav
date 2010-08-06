#ifndef VIDEOLISTENER_H_
#define VIDEOLISTENER_H_

/*
 * @file VideoListener.h
 * Definition of the VideoListener class, which defines behavior on receiving
 * incoming video streams, deleting video streams, and receiving RTCP data.
 * @author Andrew Ford
 */

#include <VPMedia/VPMSession.h>

#include <sys/time.h>

class gravManager;
class VPMVideoSink;
class GLCanvas;
class wxStopWatch;

static void newFrameCallbackTest( VPMVideoSink* sink, int buffer_idx,
                                void* user_data );
 
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

private:
    gravManager* grav;
    wxStopWatch* timer;
    
    // initial starting points for videos
    float x;
    float y;
    
};

#endif /*VIDEOLISTENER_H_*/
