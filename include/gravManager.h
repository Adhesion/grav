/*
 * @file gravManager.h
 *
 * Header file for major grav functions - holds lists of drawn objects and
 * sources, draws them, executes functions like automatic mode, etc.
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

#ifndef GRAVMANAGER_H_
#define GRAVMANAGER_H_

#include <wx/wx.h>
#include <vector>
#include <map>

#include "RectangleBase.h"
#include "GLCanvas.h"

#include <VPMedia/thread_helper.h>

class VideoSource;
class Group;
class VideoListener;
class AudioManager;
class VPMSession;
class VPMSessionFactory;
class Earth;
class InputHandler;
class TreeControl;
class LayoutManager;
class Runway;
class VenueClientController;
class SessionManager;
class Camera;
class Point;

class gravManager
{

public:
    gravManager();
    ~gravManager();

    /*
     * The main draw function: draws all objects in drawnObjects, as well as
     * calling the draw functions for the earth and the earth-video lines.
     */
    void draw();

    void clearSelected();
    void ungroupAll();

    void addTestObject();
    /*
     * Delete a user-deletable object. For now, only used for test objects.
     */
    void tryDeleteObject( RectangleBase* obj );

    /*
     * For moving videos to the top of the drawnObjects (for both drawing and
     * selection).
     * Bool val is used when checking the object's group to prevent infinite
     * loops - group checking is done to move groups to top when moving group
     * members to top.
     * Note these are NOT thread-safe, lockSources() should be called around
     * them.
     */
    void moveToTop( RectangleBase* object, bool checkGrouping = true );
    void moveToTop( std::vector<RectangleBase*>::iterator i,
                        bool checkGrouping = true );

    void drawCurvedEarthLine( float lat, float lon,
                              float destx, float desty, float destz );
    void drawEarthPoint( float lat, float lon, float size );

    void setBoxSelectDrawing( bool draw );
    int getWindowWidth(); int getWindowHeight();
    void setWindowWidth( int w ); void setWindowHeight( int h );
    void setWindowSize( int w, int h );

    /*
     * Are siteID groups being used? (getter/setter)
     */
    bool usingSiteIDGroups();
    void setSiteIDGrouping( bool site );

    /*
     * Access the counter used to control the drawing of the selection box.
     * (Needed for a deselect check in InputHandler)
     */
    int getHoldCounter();

    void resetAutoCounter();

    /*
     * Scale selected objects by a certian amount.
     */
    void scaleSelectedObjects( float scaleAmt );

    /*
     * Getters for accessing sources/objects. (Note these are actual lifetime,
     * non-ephemeral lists, ie this class will keep these around and modify
     * them)
     */
    std::vector<VideoSource*>* getSources();
    std::vector<RectangleBase*>* getDrawnObjects();
    std::vector<RectangleBase*>* getSelectedObjects();
    std::map<std::string,Group*>* getSiteIDGroups();

    /*
     * "Movable" being defined as selectable non-groups, ie, things that will
     * be moved by the user-initiated arrangements.
     */
    std::vector<RectangleBase*> getMovableObjects();
    /*
     * Note that this is actually a subset of the movable objects, meaning it's
     * not EVERY unselected object, just ones that could be moved.
     */
    std::vector<RectangleBase*> getUnselectedObjects();

    /*
     * Manage sources in the main list of sources as well as in the lists of
     * drawn & selected objects.
     * Latter is an iterator so as not to have to loop through the list of
     * sources twice - mainly because the delete-by-ssrc in videolistener
     * needs to find its target that way.
     * I.e., delete expects that the caller will find the source in the
     * sources list itself.
     */
    void addNewSource( VideoSource* s );
    void deleteSource( std::vector<VideoSource*>::iterator si );
    void deleteGroup( Group* g );

    /*
     * Note these are NOT thread-safe, lockSources() should be called around
     * them.
     */
    void addToDrawList( RectangleBase* obj );
    void removeFromLists( RectangleBase* obj, bool treeRemove = true );

    /*
     * Creates a group for siteID-based grouping, with the data string as the
     * name, adds it to the list, and returns a pointer to it.
     * Note this is NOT thread-safe, lockSources() should be called around this.
     */
    Group* createSiteIDGroup( std::string data );

    float getCamX(); float getCamY(); float getCamZ();
    void setCamX( float x ); void setCamY( float y ); void setCamZ( float z );
    void resetCamPosition();

    RectangleBase getScreenRect( bool full = false );
    RectangleBase getEarthRect();

    void setEarth( Earth* e );
    void setInput( InputHandler* i );
    void setTree( TreeControl* t );
    void setAudio( AudioManager* a );
    void setVideoListener( VideoListener* v );
    void setVenueClientController( VenueClientController* vcc );
    void setSessionManager( SessionManager* s );
    /*
     * Note, this should be called after GL setup since it needs to calculate
     * the text size, which depends on the font being set up.
     */
    void setHeaderString( std::string h );

    TreeControl* getTree();

    void lockSources();
    void unlockSources();

    void setThreads( bool threads );

    bool usingRunway();
    bool usingGridAuto();
    bool usingAutoFocusRotate();
    void setRunwayUsage( bool run );
    void setGridAuto( bool g );
    void setAutoFocusRotate( bool a );
    Runway* getRunway();

    void setGraphicsDebugMode( bool g );
    bool getGraphicsDebugMode();

    void toggleShowVenueClientController();
    bool isVenueClientControllerShown();
    bool isVenueClientControllerShowable();

private:
    /*
     * Delete video sources set to be deleted. This should ONLY be called from
     * the main thread (ie, in draw()) since that's the whole point of having
     * this - we need to do VideoSource deletes on the main thread since it does
     * a GL call to delete its texture.
     */
    void doDelayedDelete();

    std::vector<VideoSource*>* sources;
    std::vector<RectangleBase*>* drawnObjects;
    std::vector<RectangleBase*>* selectedObjects;
    std::map<std::string,Group*>* siteIDGroups;

    std::vector<RectangleBase*>* objectsToDelete;
    std::vector<RectangleBase*>* objectsToAddToTree;
    std::vector<RectangleBase*>* objectsToRemoveFromTree;

    // temp lists for doing auto/audio focus
    std::vector<RectangleBase*> outerObjs;
    std::vector<RectangleBase*> innerObjs;

    LayoutManager* layouts;

    Runway* runway;

    Camera* cam;

    // these following block of pointers are NOT owned by this class

    Earth* earth;

    InputHandler* input;

    TreeControl* tree; // we need a reference to the GUI tree so we can
                       // update the names and add new objects

    // mostly just to grab the number of sources for arrangement purposes
    VideoListener* videoListener;

    VenueClientController* venueClientController;

    SessionManager* sessionManager;

    std::string headerString;
    bool useHeader;
    FTBBox headerTextBox;
    float textScale;
    float textOffset;

    AudioManager* audio;
    bool audioFocusTrigger;
    // "audioEnabled" only means that the AudioManager object is available, not
    // that there actually is any audio being used in the session.
    // audioAvailable() accomplishes this by checking number of sources in
    // AudioManager
    bool audioEnabled;
    bool audioAvailable();

    bool drawSelectionBox;

    // dimensions of the drawing window in pixels
    int windowWidth, windowHeight;
    // rectangle that represents the boundaries of the drawing area in world
    // space - full and one minus header & runway
    RectangleBase screenRectFull;
    RectangleBase screenRectSub;
    // rectangle that roughly defines where the earth is relative to the camera
    RectangleBase earthRect;
    void recalculateRectSizes();

    int holdCounter;
    int drawCounter;
    int autoCounter;
    int intersectCounter;
    bool enableSiteIDGroups;

    bool usingThreads;
    mutex* sourceMutex; // this is owned by us
    int lockCount;

    bool useRunway;
    bool gridAuto;
    bool autoFocusRotate;

    bool graphicsDebugView;
    long pixelCount;

};

#endif /*GRAVMANAGER_H_*/
