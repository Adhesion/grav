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

#include "VideoSource.h"
#include "glutil.h"

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
static std::vector<VPMVideoBufferSink*> session_sinks;
static std::vector<VPMVideoBufferSink*>::iterator session_sink_current;
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

// start & end pos for click-and-dragging
static float dragStartX;
static float dragStartY;
static float dragEndX;
static float dragEndY;

// for checking click-and-drag
static bool lastBoxed; // was our last selection a boxed selection?
static bool leftButtonHeld;

static void glutDisplay(void);
static void glutReshape(int w, int h);
static void glutKeyboard(unsigned char key, int x, int y);
static void glutIdle(void);
static void glutTimer(int v);
static void glutMouse(int button, int state, int x, int y);
static void glutActiveMotion(int x, int y);

static bool selectVideos( bool box );
static void clearSelected();
static void retileVideos();

static std::vector<VideoSource*> sources;
static std::vector<VideoSource*> selectedSources;

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
  session_sink_current = session_sinks.end();

  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(windowWidth, windowHeight);
  glutInitWindowPosition(0, 50);
  glutCreateWindow(argv[0]);

  glutDisplayFunc(glutDisplay);
  glutReshapeFunc(glutReshape);
  glutKeyboardFunc(glutKeyboard);
  glutTimerFunc(33, glutTimer, 33);
  glutIdleFunc(glutIdle);
  glutMouseFunc(glutMouse);
  glutMotionFunc(glutActiveMotion);

  glutMainLoop();
  return 0;
}

static void glutDisplay(void)
{
    glClear(GL_COLOR_BUFFER_BIT);

    // iterate through all sources and draw here
    std::vector<VideoSource*>::const_iterator si;
    for ( si = sources.begin(); si != sources.end(); si++ )
    {
	    (*si)->draw();
    }
  
    // draw the click-and-drag selection box
    if ( leftButtonHeld )
    {
	    glEnable(GL_BLEND);
	    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	    glBegin(GL_QUADS);
	    
	    glColor4f( 0.1f, 0.2f, 1.0f, 0.25f );
	    
	    glVertex3f(dragStartX, dragStartY, 0.0f);
	    glVertex3f(dragEndX, dragStartY, 0.0f);
	    glVertex3f(dragEndX, dragEndY, 0.0f);
	    glVertex3f(dragStartX, dragEndY, 0.0f);
	    
	    glEnd();
	    glDisable(GL_BLEND);
    }

    glutSwapBuffers();
}

static void glutKeyboard(unsigned char key, int x, int y)
{
  std::vector<VPMVideoBufferSink*>::iterator t;
  printf( "Char pressed is %c\n", key );
  printf( "x,y in glutkeyboard is %i,%i\n", x, y );
  std::vector<VideoSource*>::const_iterator si;
  // how much to scale when doing -/+: flipped in the former case
  float scaleAmt = 0.25f;

  switch(key) {

  case 'a':
    if (session_sink_current != session_sinks.begin()) {
      session_sink_current --;
    }
    break;

  case 's':
    t = session_sink_current;
    session_sink_current ++;
    if (session_sink_current == session_sinks.end()) {
      session_sink_current = t;
    }
    break;
    
  case 'r':
    retileVideos();
    break;
    
  case 'w':
    printf( "We currently have %i sources.\n", sources.size() );
    
    for ( si = sources.begin(); si != sources.end(); si++ )
      {
        printf( "name: %s\n", (*si)->getMetadata(VPMSession::VPMSESSION_SDES_NAME).c_str() );
        printf( "\tpos (world): %f,%f\n", (*si)->getX(), (*si)->getY() );
        GLdouble scrX; GLdouble scrY; GLdouble scrZ;
        worldToScreen( (GLdouble)(*si)->getX(), (GLdouble)(*si)->getY(), 
                        (GLdouble)(*si)->getZ(), &scrX, &scrY, &scrZ);
        printf( "\tpos (screen): %f,%f,%f\n", scrX, scrY, scrZ );
      }
    break;
    
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
        (*si)->moveX(0.0f);
        (*si)->moveY(0.0f);
    }
    break;

  case ',':
    //angle -= 5.0;
    break;
    
  case '.':
    //angle += 5.0;
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

  case 'q':
  case 27:
    exit(0);
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

    glFrustum(-screen_width/10.0, screen_width/10.0, -screen_height/10.0, screen_height/10.0, 0.1, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0, 0.0, 10.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
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
    if ( state == GLUT_DOWN && button == GLUT_LEFT_BUTTON )
    {
        // glut screen coords are y-flipped relative to GL screen coords
        y = windowHeight - y;
        
        // get world coords for current mouse pos
        screenToWorld( (GLdouble)x, (GLdouble)y, 0.990991f, &mouseX, &mouseY,
                        &mouseZ );
        
        printf( "mouse clicked at world %f,%f; screen %i,%i\n",
                mouseX, mouseY, x, y );
        
        // on click, any potential dragging afterwards must start here
   	    dragStartX = mouseX;
   	    dragStartY = mouseY;
        
        // if we didn't click on a video, move the selected video(s) to
        // the clicked position in empty space
        if ( !selectVideos( false ) )
        {
            if ( !selectedSources.empty() )
            {
	            printf( "moving videos...\n" );
	            std::vector<VideoSource*>::const_iterator sli;
                float movePosX = mouseX; float movePosY = mouseY;
                for ( sli = selectedSources.begin(); 
                       sli != selectedSources.end();
                       sli++ )
	            {
	                (*sli)->moveX( movePosX );
	                (*sli)->moveY( movePosY );
	                (*sli)->setSelect( false );
	                
	                // check & set the position difference between videos
	                // so that they're not all moved to the same position
	                std::vector<VideoSource*>::const_iterator next = sli+1;
	                if ( next != selectedSources.end() )
	                {
	                    movePosX += (*next)->getX() - (*sli)->getX();
	                    movePosY += (*next)->getY() - (*sli)->getY();
	                }
	            }
	            selectedSources.clear();
            }
            else
                leftButtonHeld = true;
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
    screenToWorld( (GLdouble)x, (GLdouble)y, 0.990991f, &mouseX, &mouseY,
                    &mouseZ );
    
    dragEndX = mouseX;
    dragEndY = mouseY;
    if ( leftButtonHeld )
        selectVideos(true);
}

bool selectVideos( bool box )
{
    int special = glutGetModifiers();
    bool videoSelected = false;
    
    std::vector<VideoSource*>::reverse_iterator si;
    // reverse means we'll get the video that's on top first, since videos
    // later in the list will render on top of previous ones
    std::vector<VideoSource*>::const_iterator sli;
    
    printf( "performing box selection? %i\n", box );
    printf( "was the last selection box? %i\n", lastBoxed );
    
    // if we don't have ctrl held down, we can only select one
    // video at a time so clear the current list of selected videos
    if ( special != GLUT_ACTIVE_CTRL && 
            (box && lastBoxed) )
        clearSelected();
    
    for ( si = sources.rbegin(); si != sources.rend(); si++ )
    {
        // find the bounds of the video
        float left = (*si)->getX() - (*si)->getWidth()/2;
        float right = (*si)->getX() + (*si)->getWidth()/2;
        float bottom = (*si)->getY() - (*si)->getHeight()/2;
        float top = (*si)->getY() + (*si)->getHeight()/2;
        
        bool clickedInside = (left < mouseX) && (mouseX < right) &&
                (bottom < mouseY) && (mouseY < top);
        if ( clickedInside && !box )
            clearSelected();
        
        bool isBoxedX = false;
        bool isBoxedY = false;
        if (box)
        {
	        if ( dragStartX < dragEndX )
	        {
	            if ( dragStartX < left && left < dragEndX )
	                isBoxedX = true;
	        }
	        else
	        {
	            if ( dragEndX < right && right < dragStartX )
	                isBoxedX = true;
	        }
	        if ( dragStartY < dragEndY )
	        {
	            if ( dragStartY < bottom && bottom < dragEndY )
	                isBoxedY = true;
	        }
	        else
	        {
	            if ( dragEndY < top && top < dragStartY )
	                isBoxedY = true;
	        }
            lastBoxed = true;
        }
        else
            lastBoxed = false;
        
        // if we clicked inside a video (on single click) or if there's
        // a video inside the bounds of the selection box...
        if ( clickedInside || (isBoxedX && isBoxedY && box) )
        {
                
            (*si)->setSelect( true );
            videoSelected = true;
            printf( "selected a video...\n" );
            
            // take the selected video and put it and the end of the list
            // so it'll be rendered on top
            std::vector<VideoSource*>::iterator cur =
                sources.begin() - 1 + distance( si, sources.rend() );
            printf( "size of sources before: %i\n", sources.size() );
            printf( "names of cur & reverse iter: %s, %s\n",
                (*cur)->getName().c_str(), (*si)->getName().c_str() );
            VideoSource* temp = (*cur);
            sources.erase( cur );
            printf( "size of sources after: %i\n", sources.size() );
            sources.push_back( temp );
            printf( "size of sources after after: %i\n", sources.size() );
            
            selectedSources.push_back( temp );
            if ( !box ) break; // so we only select one video per click
                               // when single-clicking
        }
    }
    
    return videoSelected;
}

void clearSelected()
{
    printf( "clearing\n" );
    for ( std::vector<VideoSource*>::iterator sli = selectedSources.begin();
            sli != selectedSources.end(); sli++ )
        (*sli)->setSelect(false);
    selectedSources.clear();
}

void retileVideos()
{
    x = -9.0f;
    y = 7.0f;
    float buffer = 1.0f; // space between videos
    
    std::vector<VideoSource*>::iterator si;
    for ( si = sources.begin(); si != sources.end(); si++ )
    {
        (*si)->moveX(x);
        (*si)->moveY(y);
        std::vector<VideoSource*>::iterator next = si + 1;
        if ( next == sources.end() ) next = sources.begin();
        x += (*si)->getWidth()/2 + buffer +
                (*next)->getWidth()/2;
        if ( x > 9.0f )
        {
            x = -9.0f;
            y -= (*si)->getHeight()/2 + buffer +
                    (*next)->getHeight()/2;
        }
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
    sources.push_back( source );
    
    // do some basic grid positions
    x += 6.0f;
    if ( x > 18.0f )
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
    for ( si = sources.begin(); si != sources.end(); si++ )
    {
        if ( (*si)->getssrc() == ssrc )
        {
            sources.erase( si );
            delete (*si);
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
  // Ignore
}
