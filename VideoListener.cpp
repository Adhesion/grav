/*
 * @file VideoListener.h
 * Implementation of the VideoListener.
 */

#include "VideoListener.h"
#include "VideoSource.h"
#include "gravManager.h"
#include "GLCanvas.h"
#include "Group.h"
#include "TreeControl.h"
#include "GLUtil.h"

#include <VPMedia/video/VPMVideoDecoder.h>
#include <VPMedia/video/VPMVideoBufferSink.h>

VideoListener::VideoListener( gravManager* g ) :
    grav( g )
{
    x = -7.5f;
    y = 5.5f;
}

void
VideoListener::vpmsession_source_created(VPMSession &session,
                    uint32_t ssrc,
                    uint32_t pt,
                    VPMPayload type,
                    VPMPayloadDecoder *decoder)
{
    VPMVideoDecoder *d = dynamic_cast<VPMVideoDecoder*>( decoder );

    if ( d )
    {
        VPMVideoFormat format = d->getOutputFormat();
        VPMVideoBufferSink *sink;
        printf( "VideoListener::vpmsession_source_created: "
                "creating source, have shaders? %i format? %i (yuv420p: %i)\n",
                GLUtil::getInstance()->useShaders(), format, VIDEO_FORMAT_YUV420 );
        if ( GLUtil::getInstance()->useShaders() && format == VIDEO_FORMAT_YUV420 )
            sink = new VPMVideoBufferSink( format );
        else
            sink = new VPMVideoBufferSink( VIDEO_FORMAT_RGB24 );

        // note that the buffer sink will be deleted when the decoder for the
        // source is (inside VPMedia), so that's why it isn't deleted here or in
        // videosource

        if ( !sink->initialise() )
        {
            fprintf(stderr, "Failed to initialise video sink\n");
            return;
        }

        d->connectVideoProcessor(sink);

        //printf( "creating new source at %f,%f\n", x, y );
        
        VideoSource* source = new VideoSource( &session, ssrc, sink, x, y );
        grav->addNewSource( source );
        
        // new frame callback mostly just used for testing
        //sink->addNewFrameCallback( &newFrameCallbackTest, (void*)timer );

        // do some basic grid positions
        // TODO make this better, use layoutmanager somehow?
        x += 8.8f;
        if ( x > 15.0f )
        {
            x = -7.5f;
            y -= 6.0f;
        }
    }
}

void 
VideoListener::vpmsession_source_deleted(VPMSession &session,
                    uint32_t ssrc,
                    const char *reason)
{
    std::vector<VideoSource*>::iterator si;
    printf( "grav: deleting ssrc 0x%08x\n", ssrc );
    for ( si = grav->getSources()->begin();
            si != grav->getSources()->end(); ++si )
    {
        if ( (*si)->getssrc() == ssrc )
        {
            grav->deleteSource( si );
            return;
        }
    }
    printf( "VideoListener::source_deleted: ERROR: ssrc not found?\n" );
}

void 
VideoListener::vpmsession_source_description(VPMSession &session,
                    uint32_t ssrc)
{
  // Ignore
}

void 
VideoListener::vpmsession_source_app(VPMSession &session, 
                uint32_t ssrc, 
                const char *app , 
                const char *data, 
                uint32_t data_len)
{
    //printf( "RTP app data received\n" );
    //printf( "app: %s\n", app );
    //printf( "data: %s\n", data );
    
    std::string appS( app, 4 );
    std::string dataS( data, data_len );
    //printf( "listener::RTCP_APP: %s,%s\n", appS.c_str(), dataS.c_str() );
    //printf( "listener::RTCP_APP: data length is %i\n", data_len );
    
    if ( appS.compare( "site" ) == 0 && grav->usingSiteIDGroups() )
    {
        grav->lockSources();

        // vic sends 4 nulls at the end of the rtcp_app string for some
        // reason, so chop those off
        dataS = std::string( dataS, 0, 32 );
        std::vector<VideoSource*>::iterator i = grav->getSources()->begin();
        
        // sometimes, if groups are enabled by default, we can get RTCP APP
        // before we get any sources added, resulting in a crash when we try
        // and dereference the sources pointer - so skip this if we don't have
        // any sources yet
        if ( grav->getSources()->size() == 0 )
        {
            grav->unlockSources();
            return;
        }
        
        while ( (*i)->getssrc() != ssrc )
        {
            ++i;
            if ( i == grav->getSources()->end() )
            {
                grav->unlockSources();
                return;
            }
        }
        
        if ( !(*i)->isGrouped() )
        {
            Group* g;
            std::map<std::string,Group*>::iterator mapi =
                                     grav->getSiteIDGroups()->find(dataS);
            
            if ( mapi == grav->getSiteIDGroups()->end() )
                g = grav->createSiteIDGroup( dataS );
            else
                g = mapi->second;
            
            (*i)->setSiteID( dataS );
            g->add( *i );
            
            // adding & removing will replace the object under its group
            grav->getTree()->removeObject( (*i) );
            grav->getTree()->addObject( (*i) );
            
            grav->getTree()->updateObjectName( g );
        }

        grav->unlockSources();
    }
}

void VideoListener::setTimer( Timer* t )
{
    timer = t;
}

static void newFrameCallbackTest( VPMVideoSink* sink, int buffer_idx,
                                void* user_data )
{
    //printf( "new frame callback\n" );

    //Timer* timer = (Timer*)user_data;
    //if ( timer ) timer->printTiming();

    //printf( "VS::newframe: diff is %d\n", diff );
}
