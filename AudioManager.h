#ifndef AUDIOMANAGER_H_
#define AUDIOMANAGER_H_

/**
 * @file AudioManager.h
 * Defines classes for connecting to a multicast address for RTP audio data
 * via VPMedia, and grabs audio level data from each source for passing to
 * other objects.
 * @author Andrew Ford
 */
 
class VPMSession;
class VPMPayloadDecoder;
class VPMAudioMeter;
#include <VPMedia/VPMSessionListener.h>
#include <VPMedia/VPMPayload.h>
#include <VPMedia/VPMTypes.h>
#include <string>
#include <vector>

typedef struct AudioSource
{
    uint32_t ssrc;
    std::string siteID;
    VPMAudioMeter* meter;
} AudioSource;

class AudioManager : public VPMSessionListener
{

public:
    AudioManager();
    ~AudioManager();
    float getLevel( std::string name = "", bool avg = false );
    float getLevelAvg( std::string name = "" );
    void printLevels();
    unsigned int getSourceCount();
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
    
private:
    std::vector<AudioSource*> sources;

};

#endif /*AUDIOMANAGER_H_*/
