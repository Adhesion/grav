/**
 * @file AudioManager.h
 * Implementation of classes for connecting to a multicast address for RTP
 * audio data via VPMedia, and grabs audio level data from each source for
 * passing to other objects.
 * @author Andrew Ford
 */

#include "AudioManager.h"
#include <VPMedia/VPMPayloadDecoder.h>
#include <VPMedia/audio/linear/VPMLinear16Decoder.h>
#include <VPMedia/audio/VPMAudioMeter.h>

AudioManager::AudioManager()
{
    
}

AudioManager::~AudioManager()
{
    
}

float AudioManager::getLevel( std::string name )
{
    // Since there might be multiple sources with the same label (ie, one
    // site sending multiple audio streams) we'll average the values of the
    // meters that match the given name.
    float temp = 0.0f;
    int count = 0;
    //printf( "AudioManager::getLevel: getting level for %s\n", name.c_str() );
    for ( unsigned int i = 0; i < sources.size(); i++ )
    {
        if ( sources[i]->siteID.compare( name ) == 0 )
        {
            //printf( "AudioManager::getLevel: found %s\n", name.c_str() );
            temp += sources[i]->meter->level();
            count++;
        }
        //else
            //printf( "AudioManager::getLevel: DID NOT find %s\n", name.c_str() );
    }
    if ( count > 0 )
        return temp/(float)count;
    // default case
    return -2.0f;
}

void AudioManager::printLevels()
{
    for ( unsigned int i = 0; i < sources.size(); i++ )
    {
        printf( "source: 0x%08x/%s: %f\n", sources[i]->ssrc,
        sources[i]->siteID.c_str(), sources[i]->meter->level() );
    }
}

float AudioManager::getLevelAvg()
{
    float temp = 0.0f;
    int count = 0;
    for ( unsigned int i = 0; i < sources.size(); i++ )
    {
        temp += sources[i]->meter->levelAverage();
        sources[i]->meter->resetAverage();
        count++;
    }
    if ( count > 0 )
        return temp/(float)count;
    return -2.0f;
}

void AudioManager::vpmsession_source_created( VPMSession &session,
                                          uint32_t ssrc,
                                          uint32_t pt,
                                          VPMPayload type,
                                          VPMPayloadDecoder *decoder )
{
    printf( "AudioManager::vpmsession_source_created: "
            "adding source ssrc: 0x%08x\n", ssrc );
    VPMLinear16Decoder* dec = dynamic_cast<VPMLinear16Decoder*>(decoder);
    if ( dec )
    {
        AudioSource* a = new AudioSource;
        VPMAudioMeter* m = new VPMAudioMeter();
        a->ssrc = ssrc;
        a->meter = m;
        
        dec->connectAudioProcessor( m );
        
        sources.push_back( a );
        printf( "AudioManager::vpmsession_source_created: source added\n" );
    }
    else
        printf( "AudioManager::vpmsession_source_created: failed to cast to"
                " linear16 decoder\n" );
}

void AudioManager::vpmsession_source_deleted( VPMSession &session,
                                          uint32_t ssrc,
                                          const char *reason )
{
    
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
    printf( "AudioManager::RTCP_APP: ssrc: 0x%08x\n", ssrc );
    printf( "AudioManager::RTCP_APP: %s,%s\n", appS.c_str(), dataS.c_str() );
    printf( "AudioManager::RTCP_APP: data length is %i\n", data_len );
    
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
