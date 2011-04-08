/*
 * @file AudioManager.h
 *
 * Defines classes for connecting to a multicast address for RTP audio data
 * via VPMedia, and grabs audio level data from each source for passing to
 * other objects.
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

#ifndef AUDIOMANAGER_H_
#define AUDIOMANAGER_H_

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
    std::string cName;
    VPMAudioMeter* meter;
    VPMSession* session;
} AudioSource;

class AudioManager : public VPMSessionListener
{

public:
    AudioManager();
    ~AudioManager();

    // avg: get running level average for single source
    // cnames: check cname instead of siteID
    // if name is blank, average all sources
    float getLevel( std::string name = "", bool avg = false,
                        bool cnames = false );
    float getLevelAvg( std::string name = "" );
    void printLevels();

    unsigned int getSourceCount();

    void updateNames();

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
