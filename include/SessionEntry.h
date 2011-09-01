/*
 * @file SessionEntry.h
 *
 * Definition of a class to encapsulate a video or audio session.
 * Also drawable (ie as a 3D GUI element, videos are separate objects), hence
 * the inheritance from RectangleBase.
 *
 * Created on: Aug 11, 2011
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

#ifndef SESSIONENTRY_H_
#define SESSIONENTRY_H_

#include "RectangleBase.h"

class VPMSessionListener;

class SessionEntry : public RectangleBase
{

public:
    SessionEntry( std::string addr, bool aud );
    ~SessionEntry();

    bool initSession( VPMSessionListener* listener );
    void disableSession();

    // note the difference between these two: if processingEnabled = false, the
    // session will still be active but not processed/iterated, but if
    // sessionEnabled = false, the session object is NULL (therefore you won't
    // be receiving any of the data)
    bool isSessionEnabled();
    void setProcessingEnabled( bool proc );
    bool isProcessingEnabled();

    bool isAudioSession();

    void setEncryptionKey( std::string key );
    void disableEncryption();
    bool isEncryptionEnabled();

    std::string getAddress();
    uint32_t getTimestamp();

    bool iterate();

private:
    std::string address;

    std::string encryptionKey;
    bool encryptionEnabled;

    bool audio;

    bool processingEnabled;
    bool initialized;

    VPMSession* session;
    uint32_t sessionTS;

};

#endif /* SESSIONENTRY_H_ */
