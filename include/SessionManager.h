/*
 * @file SessionManager.h
 *
 * Definition of a class for managing audio/video sessions - adding, removing,
 * disabling, rotating through many, etc. Thread-safe.
 * This is the lower-level code - any clients should call the relevant methods
 * in SessionTreeControl in order to keep the GUI synced up.
 *
 * Created on: Sep 28, 2010
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

#ifndef SESSIONMANAGER_H_
#define SESSIONMANAGER_H_

class VPMSession;
class VideoListener;
class AudioManager;
class mutex;

#include <vector>

#include <VPMedia/VPMTypes.h>

typedef struct {
    std::string address;
    std::string encryptionKey;
    bool encryptionEnabled;
    bool audio;
    bool enabled;
    VPMSession* session;
    uint32_t sessionTS;
} SessionEntry;

class SessionManager
{

public:
    SessionManager( VideoListener* vl, AudioManager* al );
    ~SessionManager();

    /*
     * Create a new RTP session and attach it to the proper listener. If
     * audio is true it's an audio session; video if false.
     * Returns false if session creation fails, true otherwise.
     */
    bool initSession( std::string addr, bool audio );
    bool removeSession( std::string addr );

    /*
     * Note audio is ignored for this for the time being - only rotating video
     * sessions for now.
     */
    void addRotatedSession( std::string addr, bool audio );
    void removeRotatedSession( std::string addr, bool audio );
    void rotate( bool audio );

    std::string getCurrentRotateSession();
    std::string getLastRotateSession();

    bool setSessionEnable( std::string addr, bool set );
    bool isSessionEnabled( std::string addr );

    bool setEncryptionKey( std::string addr, std::string key );
    bool disableEncryption( std::string addr );
    bool isEncryptionEnabled( std::string addr );

    /*
     * Returns true if there were enabled sessions to iterate.
     */
    bool iterateSessions();

    int getVideoSessionCount();
    int getAudioSessionCount();

    void lockSessions();
    void unlockSessions();

private:
    std::vector<SessionEntry> sessions;
    VideoListener* videoSessionListener;
    AudioManager* audioSessionListener;
    int videoSessionCount;
    int audioSessionCount;

    std::vector<std::string> videoRotateList;
    int rotatePos;
    std::string lastRotateSession;

    mutex* sessionMutex;
    int lockCount;
    bool pause;

};

#endif /* SESSIONMANAGER_H_ */
