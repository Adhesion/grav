#ifndef GLUTVIDEO_H_
#define GLUTVIDEO_H_

/**
 * @file glutVideo.h
 * Header file for main grav functions.
 * @author Andrew Ford
 */

#include <wx/wx.h>
#include <vector>
#include <map>

#include "GLCanvas.h"

static void glutDisplay(void);
static void glutReshape(int w, int h);
static void glutIdle(void);
static void glutTimer(int v);

static void glutKeyboard(unsigned char key, int x, int y);
static void glutSpecialKey(int key, int x, int y);
static void glutMouse(int button, int state, int x, int y);
static void glutActiveMotion(int x, int y);

class RectangleBase;
class VideoSource;
class Group;
class VideoListener;
class AudioManager;
class VPMSession;
class VPMSessionFactory;
class Earth;
class InputHandler;

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
    
    /*
     * For moving videos to the top of the drawnObject (for both drawing and
     * selection)
     */
    void moveToTop( RectangleBase* object );
    void moveToTop( std::vector<RectangleBase*>::iterator i );
    
    void drawCurvedEarthLine( float lat, float lon, 
                              float destx, float desty, float destz );
                                    
    void setBoxSelectDrawing( bool draw );
    int getWindowWidth(); int getWindowHeight();
    void setWindowWidth( int w ); void setWindowHeight( int h );
    
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
    
    void setEarth( Earth* e );
    void setInput( InputHandler* i );
                                    
private:
    
    std::vector<VideoSource*>* sources;
    std::vector<RectangleBase*>* drawnObjects;
    std::vector<RectangleBase*>* selectedObjects;
    std::map<std::string,Group*>* siteIDGroups;
    
    Earth* earth;

    InputHandler* input;
    
    VPMSessionFactory *sf;
    
    VPMSession *videoSession;
    uint32_t videoSession_ts;
    VideoListener* videoSession_listener;
    
    bool audioEnabled;
    VPMSession *audioSession;
    uint32_t audioSession_ts;
    AudioManager* audioSession_listener;

    bool drawSelectionBox;
    
    // dimensions of the drawing window in pixels
    int windowWidth, windowHeight;
    
    // background texture for groups & video objects
    GLuint borderTex;
    int borderWidth;
    int borderHeight;
    
    int holdCounter;
    bool enableSiteIDGroups;
    
    float camX;
    float camY;
    float camZ;
    
};

#endif /*GLUTVIDEO_H_*/
