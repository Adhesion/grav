#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <VPMedia/VPMSession.h>
#include <VPMedia/VPMSessionFactory.h>
#include <VPMedia/video/VPMVideoDecoder.h>
#include <VPMedia/video/VPMVideoBufferSink.h>
#include <VPMedia/video/format.h>
#include <VPMedia/random_helper.h>
#include <VPMedia/VPMLog.h>

#include <cstdlib>
#include <vector>
#include <iostream>
#include <algorithm>

#include "VideoSource.h"
#include "GLUtil.h"
#include "RectangleBase.h"
#include "Group.h"
#include "Earth.h"
#include "PNGLoader.h"

class listener : public VPMSessionListener {
public:
  listener();
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
};
   

static VPMSession *session;
static uint32_t session_ts;
static listener session_listener;

static float screen_width;
static float screen_height;

// dimensions of the glut window in pixels
static int windowWidth = 800;
static int windowHeight = 600;

// initial starting position for the first video
static float x = -7.5f;
static float y = 5.0f;

// mouse pos
static GLdouble mouseX, mouseY, mouseZ;

static float camX = 0.0f;
static float camY = 0.0f;
static float camZ = 9.0f;

// start & end pos for click-and-dragging
static float dragStartX;
static float dragStartY;
static float dragEndX;
static float dragEndY;

// for checking click-and-drag
static bool lastBoxed; // was our last selection a boxed selection?
static bool leftButtonHeld;
static bool clickedInside;
static int holdCounter;
static bool drawSelectionBox;

// for enabling siteID-based groups
static bool enableSiteIDGroups;

// special input modifiers like CTRL
static int special;

// background texture for groups & video objects
static GLuint borderTex;
int borderWidth;
int borderHeight;

static void glutDisplay(void);
static void glutReshape(int w, int h);
static void glutKeyboard(unsigned char key, int x, int y);
static void glutSpecialKey(int key, int x, int y);
static void glutIdle(void);
static void glutTimer(int v);
static void glutMouse(int button, int state, int x, int y);
static void glutActiveMotion(int x, int y);

static bool selectVideos( bool box );
static void clearSelected();
static void ungroupAll();
static void retileVideos();

// for moving videos to the top of the drawnObject (for both drawing and
// selection)
static void moveToTop( RectangleBase* object );
static void moveToTop( std::vector<RectangleBase*>::iterator i );

static std::vector<VideoSource*> sources;
static std::vector<RectangleBase*> drawnObjects;
static std::vector<RectangleBase*> selectedObjects;
static std::vector<RectangleBase*> tempSelectedObjects;
static std::map<std::string,Group*> siteIDGroups;

static Earth* earth;

int main(int argc, char *argv[])
{
  glutInit(&argc, argv);

  if (argc != 2) {
    fprintf(stderr, "error: usage %s <ipaddr/port>\n", argv[0]);
    return -1;
  }

  vpmlog_set_log_level( VPMLOG_LEVEL_DEBUG );

  VPMSessionFactory *sf = VPMSessionFactory::getInstance();

  session = sf->createSession(argv[1], session_listener);

  session->enableVideo(true);
  session->enableAudio(false);
  session->enableOther(true);

  if (!session->initialise()) {
    fprintf(stderr, "error: failed to initialise session\n");
    return -1;
  }

  session_ts = random32();

  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(windowWidth, windowHeight);
  glutInitWindowPosition(0, 50);
  glutCreateWindow(argv[0]);
  
  earth = new Earth();
  borderTex = PNGLoader::loadPNG( "border.png", borderWidth, borderHeight );

  glutDisplayFunc(glutDisplay);
  glutReshapeFunc(glutReshape);
  glutKeyboardFunc(glutKeyboard);
  glutSpecialFunc(glutSpecialKey);
  glutTimerFunc(33, glutTimer, 33);
  glutIdleFunc(glutIdle);
  glutMouseFunc(glutMouse);
  glutMotionFunc(glutActiveMotion);
  
  glEnable( GL_DEPTH_TEST );

  glutMainLoop();
  return 0;
}

static void glutDisplay(void)
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    glLoadIdentity();
    gluLookAt(camX, camY, camZ, 0.0, 0.0, -25.0, 0.0, 1.0, 0.0);
    
    earth->draw();
    
    // lat/long testing
    float sx, sy, sz;
    
    earth->convertLatLong( 0.0f, 0.0f, sx, sy, sz );
    glColor3f( 1.0f, 0.0f, 0.0f );
    glBegin( GL_LINE );
    glVertex3f( 0.0f, 0.0f, 0.0f );
    glVertex3f( sx, sy, sz );
    glEnd();
    
    earth->convertLatLong( 43.165556f, -77.611389f, sx, sy, sz );
    glColor3f( 0.0f, 0.0f, 1.0f );
    glBegin( GL_LINE );
    glVertex3f( 0.0f, 0.0f, 0.0f );
    glVertex3f( sx, sy, sz );
    glEnd();
    
    earth->convertLatLong( 51.507778f, -0.128056f, sx, sy, sz );
    glColor3f( 0.0f, 1.0f, 0.0f );
    glBegin( GL_LINE );
    glVertex3f( 0.0f, 0.0f, 0.0f );
    glVertex3f( sx, sy, sz );
    glEnd();
    
    // iterate through all sources and draw here
    // polygon offset to fix z-fighting of coplanar polygons (videos)
    glEnable( GL_POLYGON_OFFSET_FILL );
    //glPolygonOffset( 0.1, 1.0 );
    
    glPolygonMode( GL_FRONT, GL_FILL );
    float pOffset = 0.1;
    
    std::vector<RectangleBase*>::const_iterator si;
    
    // this makes the depth buffer read-only for this bit - this prevents
    // z-fighting on the videos which are coplanar
    glDepthMask( GL_FALSE );
    for ( si = drawnObjects.begin(); si != drawnObjects.end(); si++ )
    {
        // only draw if not grouped - groups are responsible for
        // drawing their members
        if ( !(*si)->isGrouped() )
        {
            //glDepthRange (0.1, 1.0);
            //glPolygonOffset( 0.1, 0.9 );
	        (*si)->draw();
            //pOffset += 1/sources.size();
            //printf( "drawing %s\n", (*si)->getName().c_str() );
        }
        else
        {
            //printf( "%s is grouped, not drawing\n", (*si)->getName().c_str() );
            //printf( "its group is %s\n", (*si)->getGroup()->getName().c_str() );
        }
    }
    // back to writeable z-buffer for proper earth/line rendering
    glDepthMask( GL_TRUE );
    
    // draw the click-and-drag selection box
    if ( holdCounter > 1 && drawSelectionBox )
    {
	    glEnable(GL_BLEND);
	    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // the main box
	    glBegin(GL_QUADS);
	    
	    glColor4f( 0.1f, 0.2f, 1.0f, holdCounter/25.0f * 0.25f );
	    
	    glVertex3f(dragStartX, dragStartY, 0.0f);
	    glVertex3f(dragEndX, dragStartY, 0.0f);
	    glVertex3f(dragEndX, dragEndY, 0.0f);
	    glVertex3f(dragStartX, dragEndY, 0.0f);
	    
	    glEnd();
        
        // the outline
        glBegin(GL_LINE_LOOP);
        
        glColor4f( 0.5f, 0.6f, 1.0f, holdCounter/25.0f * 0.25f );
        
        glVertex3f(dragStartX, dragStartY, 0.0f);
        glVertex3f(dragEndX, dragStartY, 0.0f);
        glVertex3f(dragEndX, dragEndY, 0.0f);
        glVertex3f(dragStartX, dragEndY, 0.0f);
        
        glEnd();
        
	    glDisable(GL_BLEND);
    }
    
    glDisable( GL_POLYGON_OFFSET_FILL );
    
    glutSwapBuffers();
    
    //printf( "holdcounter is %i\n", holdCounter );
    if ( !leftButtonHeld && holdCounter > 0 )
        holdCounter-=2;
}

static void glutKeyboard(unsigned char key, int x, int y)
{
  std::vector<VPMVideoBufferSink*>::iterator t;
  std::map<std::string,Group*>::iterator mapi;
  printf( "Char pressed is %c\n", key );
  printf( "x,y in glutkeyboard is %i,%i\n", x, y );
  std::vector<VideoSource*>::const_iterator si;
  // how much to scale when doing -/+: flipped in the former case
  float scaleAmt = 0.25f;

  switch(key) {

  case 'k':
    printf( "current sources selected: %i\n", selectedObjects.size() );
    for ( unsigned int i = 0; i < selectedObjects.size(); i++ )
    {
        printf( "%s\n", selectedObjects[i]->getName().c_str() );
    }
    break;
  
  case 't':
    printf( "rearranging groups...\n" );
    for ( unsigned int i = 0; i < selectedObjects.size(); i++ )
    {
        Group* g = dynamic_cast<Group*>(selectedObjects[i]);
        if ( g != NULL )
        {
            g->rearrange();
        }
    }
    break;
    
  case 'u':
    printf( "updating group names...\n" );
    mapi = siteIDGroups.begin();
    for ( ; mapi != siteIDGroups.end(); mapi++ )
    {
        mapi->second->updateName();
    }
    break;
    
  case 'r':
    retileVideos();
    break;
    
  case 'l':
    printf( "We currently have %i sources.\n", sources.size() );
    printf( "We currently have %i objects in drawnObjects.\n",
                 drawnObjects.size() );
    
    for ( si = sources.begin(); si != sources.end(); si++ )
      {
        printf( "name: %s\n", 
               (*si)->getMetadata(VPMSession::VPMSESSION_SDES_NAME).c_str() );
        printf( "cname: %s\n", 
               (*si)->getMetadata(VPMSession::VPMSESSION_SDES_CNAME).c_str() );
        printf( "loc: %s\n", 
               (*si)->getMetadata(VPMSession::VPMSESSION_SDES_LOC).c_str() );
        printf( "\tpos (world): %f,%f\n", (*si)->getX(), (*si)->getY() );
        GLdouble scrX; GLdouble scrY; GLdouble scrZ;
        GLUtil::worldToScreen( (GLdouble)(*si)->getX(),
                                (GLdouble)(*si)->getY(), 
                                (GLdouble)(*si)->getZ(),
                                &scrX, &scrY, &scrZ);
        printf( "\tpos (screen): %f,%f,%f\n", scrX, scrY, scrZ );
        printf( "\tis grouped? %i\n", (*si)->isGrouped() );
      }
    break;
    
  case 'o':
    printf( "random32: %i\n", random32() );
    printf( "random32max: %i\n", random32_max() );
    
  case 'n':
    for ( si = sources.begin(); si != sources.end(); si++ )
    {
        if ( (*si)->isSelected() )
            (*si)->scaleNative();
    }
    break;
    
  case '0':
    for ( si = sources.begin(); si != sources.end(); si++ )
    {
        (*si)->move(0.0f,0.0f);
    }
    break;
    
  case 'g':
    if ( enableSiteIDGroups )
    {
        enableSiteIDGroups = false;
        ungroupAll();
    }
    else
        enableSiteIDGroups = true;
    break;
    
  case '-':
    scaleAmt *= -1.0f;
  case '+':
    for ( si = sources.begin(); si != sources.end(); si++ )
    {
        if ( (*si)->isSelected() )
        {
            (*si)->setScale( (*si)->getScaleX()+(*si)->getScaleX()*scaleAmt,
                             (*si)->getScaleY()+(*si)->getScaleY()*scaleAmt );
        }
    }
    break;
    
  case 'w':
    camZ--;
    break;
  case 's':
    camZ++;
    break;
  case 'a':
    camX--;
    break;
  case 'd':
    camX++;
    break;

  case 'q':
  case 27:
    exit(0);
    break;
  }
  
}

void glutSpecialKey( int key, int x, int y )
{
    switch( key )
    {
        case GLUT_KEY_LEFT:
            earth->rotate( 0.0f, 0.0f, -2.0f );
            break;
        case GLUT_KEY_RIGHT:
            earth->rotate( 0.0f, 0.0f, 2.0f );
            break;
        case GLUT_KEY_UP:
            earth->rotate( -2.0f, 0.0f, 0.0f );
            break;
        case GLUT_KEY_DOWN:
            earth->rotate( 2.0f, 0.0f, 0.0f );
            break;
    }
}

void glutIdle(void)
{
    session->iterate(session_ts ++);
}

void glutTimer(int ms)
{
  glutPostRedisplay();

  glutTimerFunc(ms, glutTimer, ms);
}

void glutReshape(int w, int h)
{
  // Work out coordinates so that 1.0x1.0 fits into window and maintains
  // aspect ratio.
  glViewport(0, 0, w, h);
  windowWidth = w;
  windowHeight = h;
  
  if (w > h) {
    screen_height = 1.0;
    screen_width = (float)w/(float)h;
  } else {
    screen_height = (float)h/(float)w;
    screen_width = 1.0;
  }

  bool perspective_mode = true;
  if (perspective_mode) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-screen_width/10.0, screen_width/10.0, 
              -screen_height/10.0, screen_height/10.0,
              0.1, 50.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(camX, camY, camZ, 0.0, 0.0, -25.0, 0.0, 1.0, 0.0);
  } else {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-screen_width, screen_width, -screen_height, screen_height);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
  }
}

void glutMouse( int button, int state, int x, int y )
{
    special = glutGetModifiers();
    
    if ( state == GLUT_DOWN && button == GLUT_LEFT_BUTTON )
    {
        // glut screen coords are y-flipped relative to GL screen coords
        y = windowHeight - y;
        
        drawSelectionBox = false;
        
        // get world coords for current mouse pos
        GLUtil::screenToWorld( (GLdouble)x, (GLdouble)y, 0.990991f,
                                &mouseX, &mouseY, &mouseZ );
        
        printf( "mouse clicked at world %f,%f; screen %i,%i\n",
                mouseX, mouseY, x, y );
        
        // on click, any potential dragging afterwards must start here
   	    dragStartX = mouseX;
   	    dragStartY = mouseY;
        
        // if we didn't click on a video, move the selected video(s) to
        // the clicked position in empty space
        if ( !selectVideos( false ) )
        {
            //clickedInside = false;
            if ( !selectedObjects.empty() )
            {
	            printf( "moving videos...\n" );
                float movePosX = mouseX; float movePosY = mouseY;
                std::vector<RectangleBase*>::const_iterator sli;
                
                float avgX = 0.0f, avgY = 0.0f;
                int num = selectedObjects.size();
                
                if ( num == 1 )
                {
                    selectedObjects[0]->move( movePosX, movePosY );
                    selectedObjects[0]->setSelect( false );
                }
                // if moving >1, center the videos around the click point
                // we need to find the average pos beforehand
                else
                {
                    for ( sli = selectedObjects.begin(); 
                           sli != selectedObjects.end();
                           sli++ )
                    {
                        avgX += (*sli)->getX();
                        avgY += (*sli)->getY();
                    }
                    avgX = avgX / num;
                    avgY = avgY / num;
                
                    for ( sli = selectedObjects.begin(); 
                           sli != selectedObjects.end();
                           sli++ )
    	            {
    	                (*sli)->move( movePosX + ((*sli)->getX()-avgX),
    	                               movePosY + ((*sli)->getY()-avgY) );
    	                (*sli)->setSelect( false );
    	            }
                }
                
	            selectedObjects.clear();
            }
            else
            {
                leftButtonHeld = true;
            }
        }
    }
    else if ( state == GLUT_UP )
    {
        //selectVideos( true );
        printf( "resetting held state to false\n" );
        leftButtonHeld = false;
    }
}

void glutActiveMotion( int x, int y )
{   
    // glut screen coords are y-flipped relative to GL screen coords
    y = windowHeight - y;
    
    // get world coords for current mouse pos
    GLUtil::screenToWorld( (GLdouble)x, (GLdouble)y, 0.990991f,
                            &mouseX, &mouseY, &mouseZ );
    
    dragEndX = mouseX;
    dragEndY = mouseY;
    
    // for click-and-drag movement
    if ( clickedInside )
    {
        std::vector<RectangleBase*>::reverse_iterator sli;
        for ( sli = selectedObjects.rbegin(); sli != selectedObjects.rend();
                sli++ )
        {
            (*sli)->move( mouseX + 
                      ((*sli)->getX() - (*selectedObjects.rbegin())->getX()),
                          mouseY + 
                      ((*sli)->getY() - (*selectedObjects.rbegin())->getY()) );
        }
        drawSelectionBox = false;
    }
    else if ( leftButtonHeld ) {//&& 
            //(selectedSources.size() == 0 || special == GLUT_ACTIVE_CTRL ) )
        clearSelected();
        selectVideos(true);
        drawSelectionBox = true;
    }
    
    if ( holdCounter < 25 ) holdCounter++;
}

bool selectVideos( bool box )
{
    bool videoSelected = false;
    
    std::vector<RectangleBase*>::reverse_iterator si;
    // reverse means we'll get the video that's on top first, since videos
    // later in the list will render on top of previous ones
    std::vector<RectangleBase*>::const_iterator sli;
    
    printf( "performing box selection? %i\n", box );
    printf( "was the last selection box? %i\n", lastBoxed );
    
    for ( si = drawnObjects.rbegin(); si != drawnObjects.rend(); si++ )
    {        
        // rectangle that defines the selection area
        float selectL, selectR, selectU, selectD;
        if (box)
        {
            selectL = std::min(dragStartX,dragEndX);
            selectR = std::max(dragStartX,dragEndX);
            selectD = std::min(dragStartY,dragEndY);
            selectU = std::max(dragStartY,dragEndY);
            lastBoxed = true;
        }
        else
        {
            selectL = selectR = mouseX;
            selectU = selectD = mouseY;
            lastBoxed = false;
        }

        bool intersect = (*si)->intersect( selectL, selectR,
                                        selectU, selectD );
                                        
        // for click-and-drag movement
        clickedInside = intersect && !leftButtonHeld;
        
        if ( intersect )
        {
            // clear the list of selected if we're clicking on a new video
            if ( clickedInside && !(*si)->isSelected() &&
                    special != GLUT_ACTIVE_CTRL )
                clearSelected();
            
            videoSelected = true;
            
            if ( !(*si)->isSelected() )
            {
                (*si)->setSelect( true );
                selectedObjects.push_back( *si );
            }
            
            // take the selected video and put it at the end of the list so
            // it'll be rendered on top - but only if we just clicked on it
            if ( !leftButtonHeld )
            {
                // since we can only delete a normal iterator (not a reverse
                // one) we have to calculate our current position
                std::vector<RectangleBase*>::iterator current =
                    drawnObjects.begin() - 1 + 
                    distance( si, drawnObjects.rend() );
                moveToTop( current );
                
                break; // so we only select one video per click
                       // when single-clicking
            }
        }
    }
    
    return videoSelected;
}

void clearSelected()
{
    printf( "clearing\n" );
    for ( std::vector<RectangleBase*>::iterator sli = selectedObjects.begin();
            sli != selectedObjects.end(); sli++ )
        (*sli)->setSelect(false);
    selectedObjects.clear();
}

void ungroupAll()
{
    printf( "deleting %i groups\n", siteIDGroups.size() );
    std::vector<RectangleBase*>::iterator it;
    for ( it = drawnObjects.begin(); it != drawnObjects.end(); )
    {
        Group* g = dynamic_cast<Group*>((*it));
        if ( g != NULL )
        {
            g->removeAll();
            it = drawnObjects.erase(it);
            
            if ( g->isSelected() )
            {
                std::vector<RectangleBase*>::iterator j =
                    selectedObjects.begin();
                while ( (*j) != g ) j++;
                selectedObjects.erase( j );
            }
            
            delete g;
            
            printf( "single group deleted\n" );
        }
        else
        {
            printf( "not a group, skipping\n" );
            it++;
        }
    }
    siteIDGroups.clear();
    printf( "siteIDgroups cleared\n" );
}

void retileVideos()
{
    x = -7.0f;
    y = 5.5f;
    float buffer = 1.0f; // space between videos
    
    std::vector<RectangleBase*>::iterator si;
    for ( si = drawnObjects.begin(); si != drawnObjects.end(); si++ )
    {
        if ( !(*si)->isGrouped() )
        {
            (*si)->move(x,y);
            std::vector<RectangleBase*>::iterator next = si + 1;
            if ( next == drawnObjects.end() ) next = drawnObjects.begin();
            x += (*si)->getWidth()/2 + buffer +
                    (*next)->getWidth()/2;
            if ( x > 8.0f )
            {
                x = -7.0f;
                y -= (*si)->getHeight()/2 + buffer +
                        (*next)->getHeight()/2;
            }
        }
        Group* g = dynamic_cast<Group*>(*si);
        if ( g != NULL )
            g->rearrange();
    }
}

void moveToTop( RectangleBase* object )
{
    std::vector<RectangleBase*>::iterator i = drawnObjects.begin();
    while ( (*i) != object ) i++;
    moveToTop( i );
}

void moveToTop( std::vector<RectangleBase*>::iterator i )
{
    RectangleBase* temp = (*i);
    drawnObjects.erase( i );
    drawnObjects.push_back( temp );
    
    Group* g = dynamic_cast<Group*>( temp );
    if ( g != NULL )
    {
        for ( int i = 0; i < g->numObjects(); i++ )
            moveToTop( (*g)[i] );
    }
}

listener::listener()
{
}

void 
listener::vpmsession_source_created(VPMSession &session,
				    uint32_t ssrc,
				    uint32_t pt,
				    VPMPayload type,
				    VPMPayloadDecoder *decoder)
{
  VPMVideoDecoder *d = dynamic_cast<VPMVideoDecoder*>(decoder);

  if (d) {
    VPMVideoBufferSink *sink = new VPMVideoBufferSink(VIDEO_FORMAT_RGB24);
    if (!sink->initialise()) {
      fprintf(stderr, "Failed to initialise video sink\n");
      return;
    }

    d->connectVideoProcessor(sink);

    printf( "creating new source at %f,%f\n", x, y );

    VideoSource* source = new VideoSource( &session, ssrc, sink, x, y );
    source->setTexture( borderTex, borderWidth, borderHeight );
    sources.push_back( source );
    source->updateName();
    drawnObjects.push_back( source );
    
    // do some basic grid positions
    x += 6.0f;
    if ( x > 9.0f )
    {
        x = -7.5f;
        y -= 6.0f;
    }
    
    //printf( "size of sources is %i\n", sources.size() );

    //session_sinks.push_back(sink);
    //session_sink_current = session_sinks.begin();
  }
}

void 
listener::vpmsession_source_deleted(VPMSession &session,
				    uint32_t ssrc,
				    const char *reason)
{
    std::vector<VideoSource*>::iterator si;
    printf( "grav: deleting ssrc 0x%08x\n", ssrc );
    for ( si = sources.begin(); si != sources.end(); si++ )
    {
        if ( (*si)->getssrc() == ssrc )
        {
            printf( "source is %s\n", (*si)->getName().c_str() );
            RectangleBase* temp = (RectangleBase*)(*si);
            std::vector<RectangleBase*>::iterator i = drawnObjects.begin();
            while ( (*i) != temp ) i++;
            drawnObjects.erase( i );
            
            if ( temp->isSelected() )
            {
                std::vector<RectangleBase*>::iterator j =
                    selectedObjects.begin();
                while ( (*j) != temp ) j++;
                selectedObjects.erase( j );
            }
            
            delete (*si);
            sources.erase( si );
            return;
        }
    }
}

void 
listener::vpmsession_source_description(VPMSession &session,
					uint32_t ssrc)
{
  // Ignore
}

void 
listener::vpmsession_source_app(VPMSession &session, 
				uint32_t ssrc, 
				const char *app , 
				const char *data, 
				uint32_t data_len)
{
    //printf( "RTP app data received\n" );
    //printf( "app: %s\n", app );
    //printf( "data: %s\n", data );
    
    std::string appS( app, 4 );
    std::string dataS( data );
    
    if ( appS.compare( "site" ) == 0 && enableSiteIDGroups )
    {
        std::vector<VideoSource*>::iterator i = sources.begin();
        //printf( "in rtcp app, got %i sources\n", sources.size() );
        while ( (*i)->getssrc() != ssrc ) 
        {
            //printf( "ssrc: %08x\n", (*i)->getssrc() );
            i++;
        
            if ( i == sources.end() )
            {
                //printf( "ssrc %08x not found?\n", ssrc );
                return;
            }
        }
        //printf( "source with ssrc %08x found\n", ssrc );
        
        if ( !(*i)->isGrouped() )
        {
            Group* g;
            std::map<std::string,Group*>::iterator mapi =
                                            siteIDGroups.find(dataS);
            
            if ( mapi == siteIDGroups.end() )
            {
                g = new Group(0.0f,0.0f);
                g->setName( dataS );
                g->setTexture( borderTex, borderWidth, borderHeight );
                drawnObjects.push_back( g );
                siteIDGroups.insert( std::pair<std::string,Group*>(dataS, g) );
            }
            else
            {
                g = mapi->second;
            }
            
            g->add( *i );
            retileVideos();
        }
    }
}
