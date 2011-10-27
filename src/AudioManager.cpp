/*
 * @file AudioManager.h
 *
 * Implementation of classes for connecting to a multicast address for RTP
 * audio data via VPMedia, and grabs audio level data from each source for
 * passing to other objects.
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

#include "AudioManager.h"
#include "gravUtil.h"

#include <VPMedia/VPMPayloadDecoder.h>
#include <VPMedia/audio/linear/VPMLinear16Decoder.h>
#include <VPMedia/audio/VPMAudioMeter.h>
#include <VPMedia/VPMSession.h>
#include <cstdio>

AudioManager::AudioManager()
{

}

AudioManager::~AudioManager()
{

}

float AudioManager::getLevel( std::string name, bool avg, bool cnames )
{
    // Since there might be multiple sources with the same label (ie, one
    // site sending multiple audio streams) we'll average the values of the
    // meters that match the given name.
    float temp = 0.0f;
    int count = 0;

    for ( unsigned int i = 0; i < sources.size(); i++ )
    {
        if ( ( !cnames && sources[i]->siteID.compare( name ) == 0 ) ||
             ( cnames && sources[i]->cName.compare( name ) == 0 ) ||
             name.compare( "" ) == 0 )
        {
            if ( avg )
            {
                temp += sources[i]->meter->levelAverage();
                sources[i]->meter->resetAverage();
            }
            else
                temp += sources[i]->meter->level();
            count++;
        }
        // would fall to else clause if name was not found
    }

    if ( count == 1 )
        return temp;
    else if ( count > 1 )
        return temp/(float)count;
    // default case
    return -2.0f;
}

float AudioManager::getLevelAvg( std::string name )
{
    return getLevel( name, true );
}

void AudioManager::printLevels()
{
    for ( unsigned int i = 0; i < sources.size(); i++ )
    {
        gravUtil::logVerbose( "AudioManager::printLevels: "
                "source: 0x%08x/%s: %f\n", sources[i]->ssrc,
                sources[i]->siteID.c_str(), sources[i]->meter->level() );
    }
}

unsigned int AudioManager::getSourceCount()
{
    return sources.size();
}

void AudioManager::updateNames()
{
    for ( unsigned int i = 0; i < sources.size(); i++ )
    {
        char buffer[256];
        uint32_t bufferLen = sizeof( buffer );

        if ( sources[i]->session->getRemoteSDES( sources[i]->ssrc,
                        VPMSession::VPMSESSION_SDES_CNAME, buffer, bufferLen ) )
        {
            sources[i]->cName = std::string( buffer );
        }
    }
}

void AudioManager::vpmsession_source_created( VPMSession &session,
                                          uint32_t ssrc,
                                          uint32_t pt,
                                          VPMPayload type,
                                          VPMPayloadDecoder *decoder )
{
    gravUtil::logVerbose( "AudioManager::vpmsession_source_created: "
            "adding source ssrc: 0x%08x\n", ssrc );
    VPMLinear16Decoder* dec = dynamic_cast<VPMLinear16Decoder*>(decoder);
    if ( dec )
    {
        AudioSource* a = new AudioSource;
        VPMAudioMeter* m = new VPMAudioMeter();
        a->ssrc = ssrc;
        a->meter = m;
        a->session = &session;

        dec->connectAudioProcessor( m );

        sources.push_back( a );
        gravUtil::logVerbose( "AudioManager::vpmsession_source_created: "
                "source added\n" );
    }
    else
        gravUtil::logWarning( "AudioManager::vpmsession_source_created: "
                "failed to cast to linear16 decoder\n" );
}

void AudioManager::vpmsession_source_deleted( VPMSession &session,
                                          uint32_t ssrc,
                                          const char *reason )
{
    gravUtil::logVerbose( "AudioManager::vpmsession_source_deleted: "
            "deleting source ssrc: 0x%08x\n", ssrc );
    std::vector<AudioSource*>::iterator it;
    for ( it = sources.begin(); it != sources.end(); ++it )
    {
        if ( (*it)->ssrc == ssrc )
        {
            delete (*it)->meter;
            delete (*it);
            sources.erase( it );
            return;
        }
    }
}

void AudioManager::vpmsession_source_description( VPMSession &session,
                                              uint32_t ssrc )
{

}

void AudioManager::vpmsession_source_app(VPMSession &session,
                                     uint32_t ssrc,
                                     const char *app,
                                     const char *data,
                                     uint32_t data_len)
{
    std::string appS( app, 4 );
    std::string dataS( data, data_len );
    gravUtil::logVerbose( "AudioManager::RTCP_APP: ssrc: 0x%08x\n", ssrc );
    gravUtil::logVerbose( "AudioManager::RTCP_APP: %s,%s\n", appS.c_str(),
            dataS.c_str() );
    gravUtil::logVerbose( "AudioManager::RTCP_APP: data length is %i\n",
            data_len );

    if ( appS.compare( "site" ) == 0 )
    {
        for ( unsigned int i = 0; i < sources.size(); i++ )
        {
            if ( sources[i]->ssrc == ssrc )
            {
                sources[i]->siteID = dataS;
            }
        }
    }
}
