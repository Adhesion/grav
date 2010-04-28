#ifndef GRAVMANAGER_H_
#define GRAVMANAGER_H_

/**
 * @file gravManager.h
 * Header file for main grav functions.
 * @author Andrew Ford
 */

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

class gravManager
{
    
public:
    gravManager();
    ~gravManager();
    
    /*
     * Create a new RTP session and attach it to the proper listener. If
     * audio is true it's an audio session; video if false.
     * Returns false if session creation fails, true otherwise.
     */
    bool initSession( std::string address, bool audio );
    
    void iterateSessions();

    /*
     * The main draw function: draws all objects in drawnObjects, as well as
     * calling the draw functions for the earth and the earth-video lines.
     */
    void draw();

    void clearSelected();
    void ungroupAll();
    void retileVideos();
    void perimeterAllVideos();
    
    void addTestObject();
    
    /*
     * For moving videos to the top of the drawnObjects (for both drawing and
     * selection)
     */
    void moveToTop( RectangleBase* object );
    void moveToTop( std::vector<RectangleBase*>::iterator i );
    
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
     * Add to the counter used to control the drawing of the selection box.
     */
    void incrementHoldCounter();
    int getHoldCounter();
    
    /*
     * Scale selected objects by a certian amount.
     */
    void scaleSelectedObjects( float scaleAmt );

    /*
     * Getters for accessing sources/objects.
     */
    std::vector<VideoSource*>* getSources();
    std::vector<RectangleBase*>* getDrawnObjects();
    std::vector<RectangleBase*>* getSelectedObjects();
    std::map<std::string,Group*>* getSiteIDGroups();
    
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
    void removeFromLists( RectangleBase* obj );

    /*
     * This would be done in the constructor, but it has to be done after
     * the GL context is set up (and in the WX case, the constructor for this
     * class has to be done before that).
     */
    void setBorderTex( std::string border );
    
    /* 
     * Creates a group for siteID-based grouping, with the data string as the
     * name, adds it to the list, and returns a pointer to it.
     */
    Group* createSiteIDGroup( std::string data );
    
    float getCamX(); float getCamY(); float getCamZ();
    void setCamX( float x ); void setCamY( float y ); void setCamZ( float z );
    
    RectangleBase getScreenRect();
    
    void setEarth( Earth* e );
    void setInput( InputHandler* i );
    void setTree( TreeControl* t );
    
    TreeControl* getTree();

    void lockSources();
    void unlockSources();

    void setThreads( bool threads );

private:
    
    std::vector<VideoSource*>* sources;
    std::vector<RectangleBase*>* drawnObjects;
    std::vector<RectangleBase*>* selectedObjects;
    std::map<std::string,Group*>* siteIDGroups;
    
    std::vector<VideoSource*>* sourcesToDelete;

    Earth* earth;

    InputHandler* input;
    
    TreeControl* tree; // we need a reference to the GUI tree so we can
                       // update the names and add new objects
    
    LayoutManager* layouts;
    
    Runway* runway;

    VPMSessionFactory *sf;
    
    VPMSession *videoSession;
    uint32_t videoSession_ts;
    VideoListener* videoSession_listener;
    bool videoInitialized;
    
    bool audioEnabled;
    VPMSession *audioSession;
    uint32_t audioSession_ts;
    AudioManager* audioSession_listener;
    bool audioInitialized;

    bool drawSelectionBox;
    
    // dimensions of the drawing window in pixels
    int windowWidth, windowHeight;
    // rectangle that represents the boundaries of the drawing area in world
    // space
    RectangleBase screenRect;
    
    // background texture for groups & video objects
    GLuint borderTex;
    int borderWidth;
    int borderHeight;
    
    int holdCounter;
    int drawCounter;
    bool enableSiteIDGroups;
    
    float camX;
    float camY;
    float camZ;
    
    bool usingThreads;
    mutex* sourceMutex;

};

#endif /*GRAVMANAGER_H_*/
