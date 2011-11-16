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
class SessionTreeControl;
class SessionGroup;
class SessionGroupButton;
class SessionEntry;
class gravManager;

#include <vector>

#include <VPMedia/VPMTypes.h>

#include "Group.h"

enum SessionType
{
    VIDEOSESSION,
    AVAILABLEVIDEOSESSION,
    AUDIOSESSION
};

class SessionManager : public Group
{

public:
    SessionManager( VideoListener* vl, AudioManager* al, gravManager* g );
    ~SessionManager();

    void rearrange();

    /*
     * Add/remove a new session. Will auto-initialize if type = video or audio.
     * Returns true if creation/removal succeeds.
     * In add's case, will return false if new session failed to
     * initialize.
     * In remove's case, will return false if session was not found.
     */
    bool addSession( std::string addr, SessionType type );
    bool removeSession( std::string addr, SessionType type );

    /*
     * Shift a session from video to available video or vice versa.
     * Type argument signifies where the session currently is, unlike the above
     * methods.
     */
    bool shiftSession( std::string addr, SessionType fromType );

    /*
     * Methods for modifying the secondary list for available video sessions.
     * Available video can be rotated through one at a time.
     * Note audio is ignored for this for the time being - only rotating video
     * sessions for now.
     */
    bool rotate( bool audio );
    bool rotateTo( std::string addr, bool audio );
    void unrotate( bool audio );

    /*
     * Sets auto-rotate (just of available video now). The timer in
     * SessionTreeControl actually triggers the rotation, this is just for
     * visual purposes (in the available video SessionGroup)
     */
    void setAutoRotate( bool a );
    void setRotateInterval( int i );

    /*
     * Method that determines what happens on SessionEntry double-click.
     * For now, only do rotate on available video sessions.
     */
    void sessionEntryAction( SessionEntry* entry );

    /*
     * Method that determines what happens on SessionGroupButton double-click.
     * For now, only trigger auto-rotate.
     */
    void sessionGroupButtonAction( SessionGroupButton* button );

    /*
     * Used for checking if user-moved SessionEntries should be shifted between
     * groups or not. Public, thread-safe interface.
     */
    void checkGUISessionShift();

    std::string getCurrentRotateSessionAddress();
    std::string getLastRotateSessionAddress();

    bool setSessionProcessEnable( std::string addr, bool set );
    bool isSessionProcessEnabled( std::string addr );

    bool isInFailedState( std::string addr, SessionType type );

    bool setEncryptionKey( std::string addr, std::string key );
    bool disableEncryption( std::string addr );
    bool isEncryptionEnabled( std::string addr );

    /*
     * Returns true if there were enabled sessions to iterate through.
     */
    bool iterateSessions();

    int getVideoSessionCount();
    int getAudioSessionCount();

    void lockSessions();
    void unlockSessions();

    void setSessionTreeControl( SessionTreeControl* s );

    /*
     * We would do this in the constructor, but it has to be put off since the
     * SessionManager is initialized before any GL stuff, texture loading etc.
     * is.
     */
    void setButtonTexture( std::string name );

    /*
     * Recalculate size based on how many sessions we have.
     */
    void recalculateSize();

private:
    /*
     * Note that all of these private functions are NOT thread safe and should
     * be enclosed in lock() calls.
     */

    /*
     * Initialize (ie, start the connection) or de-initialize a session.
     */
    bool initSession( SessionEntry* session );
    void disableSession( SessionEntry* session );

    /*
     * Finds session by address. In cases of duplicate address, will find the
     * first one. (order of video -> available video -> audio)
     * These are also not thread safe. (mostly since they get called by other
     * functions, inside their own lock()s)
     */
    SessionEntry* findSessionByAddress( std::string address );
    SessionEntry* findSessionByAddress( std::string address, SessionType type );
    int indexOf( SessionEntry* entry );

    /*
     * Internal non-thread-safe implementation of shift, so we can call it from
     * multiple places.
     */
    bool shiftSession( SessionEntry* entry );

    /*
     * The only reason we need this is for when entries get double clicked on -
     * we need to make sure the rotate call originates from the tree so its
     * display gets updated correctly.
     */
    SessionTreeControl* sessionTree;

    SessionGroup* videoSessions;
    SessionGroup* availableVideoSessions;
    SessionGroup* audioSessions;

    SessionGroupButton* avButton;

    std::map<SessionType, Group*> sessionMap;

    gravManager* objectManager;

    VideoListener* videoSessionListener;
    AudioManager* audioSessionListener;
    int videoSessionCount;
    int audioSessionCount;

    int rotatePos;
    SessionEntry* lastRotateSession;

    mutex* sessionMutex;
    int lockCount;
    bool pause;

};

#endif /* SESSIONMANAGER_H_ */
