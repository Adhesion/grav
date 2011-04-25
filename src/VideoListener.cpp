/*
 * @file VideoListener.h
 *
 * Implementation of the VideoListener.
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

#include "VideoListener.h"
#include "VideoSource.h"
#include "gravManager.h"
#include "GLCanvas.h"
#include "Group.h"
#include "TreeControl.h"
#include "GLUtil.h"
#include "gravUtil.h"

#include <VPMedia/video/VPMVideoDecoder.h>
#include <VPMedia/video/VPMVideoBufferSink.h>

#include <wx/wx.h>

VideoListener::VideoListener( gravManager* g ) :
    grav( g )
{
    initialX = -7.5f;
    initialY = 5.7f;
    x = initialX;
    y = initialY;

    sourceCount = 0;
    pixelCount = 0;
}

void VideoListener::vpmsession_source_created( VPMSession &session,
        uint32_t ssrc, uint32_t pt, VPMPayload type,
        VPMPayloadDecoder* decoder )
{
    VPMVideoDecoder *d = dynamic_cast<VPMVideoDecoder*>( decoder );

    if ( d )
    {
        sourceCount++;
        VPMVideoFormat format = d->getOutputFormat();
        VPMVideoBufferSink *sink;

        // if we have shaders available, set the output format to YUV420P so
        // the videosource class will apply the YUV420P -> RGB conversion shader
        gravUtil::logVerbose( "VideoListener::vpmsession_source_created: "
                "creating source, have shaders? %i format? %i (yuv420p: %i)\n",
                GLUtil::getInstance()->areShadersAvailable(), format,
                VIDEO_FORMAT_YUV420 );
        if ( GLUtil::getInstance()->areShadersAvailable() &&
                format == VIDEO_FORMAT_YUV420 )
            sink = new VPMVideoBufferSink( format );
        else
            sink = new VPMVideoBufferSink( VIDEO_FORMAT_RGB24 );

        // note that the buffer sink will be deleted when the decoder for the
        // source is (inside VPMedia), so that's why it isn't deleted here or in
        // videosource

        if ( !sink->initialise() )
        {
            gravUtil::logError( "VideoListener::vpmsession_source_created: "
                    "Failed to initialise video sink\n" );
            return;
        }

        d->connectVideoProcessor(sink);

        VideoSource* source = new VideoSource( &session, this, ssrc, sink, x,
													y );
        grav->addNewSource( source );

        // new frame callback mostly just used for testing
        //sink->addNewFrameCallback( &newFrameCallbackTest, (void*)timer );

        // do some basic grid positions
        // TODO make this better, use layoutmanager somehow?
        x += 8.8f;
        if ( x > 15.0f )
        {
            x = -7.5f;
            y -= 5.9f;
        }
        // reset to top
        if ( y < -11.0f )
        {
            x = initialX + ( 0.5f * ( sourceCount / 9 ) );
            y = initialY - ( 0.5f * ( sourceCount / 9 ) );
        }
    }
}

void VideoListener::vpmsession_source_deleted( VPMSession &session,
        uint32_t ssrc, const char *reason)
{
    gravUtil::logVerbose( "VideoListener::deleting ssrc 0x%08x\n", ssrc );
    std::vector<VideoSource*>::iterator si;
    for ( si = grav->getSources()->begin();
            si != grav->getSources()->end(); ++si )
    {
        if ( &session == (*si)->getSession() && (*si)->getssrc() == ssrc )
        {
            gravUtil::logVerbose( "VideoListener::found ssrc as source"
                    " 0x%08x\n", (*si) );
            sourceCount--;
            updatePixelCount( -( (*si)->getVideoWidth() *
                                 (*si)->getVideoHeight() ) );
            grav->deleteSource( si );
            return;
        }
    }
    // seems to get a lot of "sources deleted but not in video sources list" on
    // exit - may be that view-only clients are listed in the session. need to
    // test more, but not that much of an issue
}

void VideoListener::vpmsession_source_description( VPMSession &session,
        uint32_t ssrc )
{
  // Ignore
}

void VideoListener::vpmsession_source_app( VPMSession &session,
        uint32_t ssrc, const char *app, const char *data, uint32_t data_len )
{
    std::string appS( app, 4 );
    std::string dataS( data, data_len );

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
            if ( grav->getTree() )
            {
                grav->getTree()->removeObject( (*i) );
                grav->getTree()->addObject( (*i) );

                grav->getTree()->updateObjectName( g );
            }
        }

        grav->unlockSources();
    }
}

void VideoListener::setTimer( wxStopWatch* t )
{
    timer = t;
}

int VideoListener::getSourceCount()
{
    return sourceCount;
}

long VideoListener::getPixelCount()
{
	return pixelCount;
}

void VideoListener::updatePixelCount( long mod )
{
	pixelCount += mod;
}

/*static void newFrameCallbackTest( VPMVideoSink* sink, int buffer_idx,
                                void* user_data )
{
    wxStopWatch* timer = (wxStopWatch*)user_data;
    if ( timer )
    {
        long time = timer->Time();
        if ( time > 38 || time < 28 ) gravUtil::logVerbose( "VideoListener::WARNING: time > 38ms or < 28ms (%lu)\n", time );
        timer->Start( 0 );
    }
}*/
