/*
 * @file SessionManager.h
 *
 * Definition of a class for managing audio/video sessions - adding, removing,
 * disabling, rotating through many, etc. Thread-safe.
 *
 * Created on: Sep 28, 2010
 * @author Andrew Ford
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

    bool setSessionEnable( std::string addr, bool set );
    bool isSessionEnabled( std::string addr );

    /*
     * Returns true if there were enabled sessions to iterate.
     */
    bool iterateSessions();

    int getVideoSessionCount();
    int getAudioSessionCount();

private:
    std::vector<SessionEntry> sessions;
    VideoListener* videoSessionListener;
    AudioManager* audioSessionListener;
    int videoSessionCount;
    int audioSessionCount;

    mutex* sessionMutex;

};

#endif /* SESSIONMANAGER_H_ */
