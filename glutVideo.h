#ifndef GLUTVIDEO_H_
#define GLUTVIDEO_H_

/**
 * @file glutVideo.h
 * Header file for main grav functions.
 * @author Andrew Ford
 */

static void glutDisplay(void);
static void glutReshape(int w, int h);
static void glutIdle(void);
static void glutTimer(int v);

static void glutKeyboard(unsigned char key, int x, int y);
static void glutSpecialKey(int key, int x, int y);
static void glutMouse(int button, int state, int x, int y);
static void glutActiveMotion(int x, int y);

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
    void retileVideos();
    
    /*
     * For moving videos to the top of the drawnObject (for both drawing and
     * selection)
     */
    void moveToTop( RectangleBase* object );
    void moveToTop( std::vector<RectangleBase*>::iterator i );
    
    void drawCurvedEarthLine( float lat, float lon, 
                                    float dx, float dy, float dz );
                                    
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
                                    
private:
    bool drawSelectionBox;
    
    // dimensions of the glut window in pixels
    int windowWidth, windowHeight;
    
    int holdCounter;
    bool enableSiteIDGroups;
    
};

#endif /*GLUTVIDEO_H_*/
