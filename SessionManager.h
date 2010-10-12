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

    /*
     * Note audio is ignored for this for the time being - only rotating video
     * sessions for now.
     */
    void addRotatedSession( std::string addr, bool audio );
    void removeRotatedSession( std::string addr, bool audio );
    void rotate( bool audio );

    bool setSessionEnable( std::string addr, bool set );
    bool isSessionEnabled( std::string addr );

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
