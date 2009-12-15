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
#include "AudioManager.h"
#include "GLUtil.h"
#include "RectangleBase.h"
#include "Group.h"
#include "Earth.h"
#include "PNGLoader.h"
#include "InputHandler.h"

#include "glutVideo.h"

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

static bool audioEnabled;
static VPMSession *audioSession;
static uint32_t audioSession_ts;
static AudioManager audioSession_listener;

static float screen_width;
static float screen_height;

// initial starting position for the first video
static float x = -7.5f;
static float y = 5.0f;

// background texture for groups & video objects
static GLuint borderTex;
int borderWidth;
int borderHeight;

gravManager* grav;

std::vector<VideoSource*>* sources;
std::vector<RectangleBase*>* drawnObjects;
std::vector<RectangleBase*>* selectedObjects;
std::map<std::string,Group*>* siteIDGroups;

Earth* earth;

InputHandler* input;

int main(int argc, char *argv[])
{
  glutInit(&argc, argv);
  grav = new gravManager();

  if (argc != 2 && argc != 3) {
    fprintf(stderr, "error: usage %s <video ipaddr/port>\n", argv[0]);
    return -1;
  }
  
  if ( argc == 3 ) audioEnabled = true;
  else audioEnabled = false;

  vpmlog_set_log_level( VPMLOG_LEVEL_DEBUG );

  VPMSessionFactory *sf = VPMSessionFactory::getInstance();

  session = sf->createSession(argv[1], session_listener);
  
  session->enableVideo(true);
  session->enableAudio(false);
  session->enableOther(false);
  
  if (!session->initialise()) {
    fprintf(stderr, "error: failed to initialise session\n");
    return -1;
  }
  
  if ( audioEnabled )
  {
      audioSession = sf->createSession(argv[2],
                            audioSession_listener);
      
      audioSession->enableVideo(false);
      audioSession->enableAudio(true);
      audioSession->enableOther(false);
      
      if (!audioSession->initialise()) {
        fprintf(stderr, "error: failed to initialise audioSession\n");
        return -1;
      }
  }

  session_ts = random32();
  audioSession_ts = random32();
  
  sources = new std::vector<VideoSource*>();
  drawnObjects = new std::vector<RectangleBase*>();
  selectedObjects = new std::vector<RectangleBase*>();
  siteIDGroups = new std::map<std::string,Group*>();

  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
  printf( "init window to %i,%i\n", grav->getWindowWidth(), grav->getWindowHeight() );
  glutInitWindowSize(grav->getWindowWidth(), grav->getWindowHeight());
  glutInitWindowPosition(0, 50);
  glutCreateWindow(argv[0]);

  glutDisplayFunc(glutDisplay);
  glutReshapeFunc(glutReshape);
  glutTimerFunc(33, glutTimer, 33);
  glutIdleFunc(glutIdle);
  
  glutKeyboardFunc(glutKeyboard);
  glutSpecialFunc(glutSpecialKey);
  glutMouseFunc(glutMouse);
  glutMotionFunc(glutActiveMotion);
  
  // note: this init needs to be done AFTER GLUT stuff, since glGetInteger
  // (for checking max tex size) will segfault if no window is created
  earth = new Earth();
  input = new InputHandler( sources, drawnObjects, selectedObjects,
                            siteIDGroups, earth, grav );
  borderTex = PNGLoader::loadPNG( "border.png", borderWidth, borderHeight );
  
  glEnable( GL_DEPTH_TEST );

  glutMainLoop();
  return 0;
}

void glutDisplay(void)
{
    grav->draw();
}

void glutIdle(void)
{
    session->iterate(session_ts ++);
    if ( audioEnabled ) audioSession->iterate(audioSession_ts++);
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
  grav->setWindowWidth( w );
  grav->setWindowHeight( h );
  
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
    gluLookAt(grav->getCamX(), grav->getCamY(), grav->getCamZ(),
              0.0, 0.0, -25.0,
              0.0, 1.0, 0.0);
  } else {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-screen_width, screen_width, -screen_height, screen_height);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
  }
}

void glutKeyboard(unsigned char key, int x, int y)
{
    input->processKeyboard( key, x, y );
}

void glutSpecialKey( int key, int x, int y )
{
    input->processSpecialKey( key, x, y );
}

void glutMouse( int button, int state, int x, int y )
{
    input->processMouse( button, state, x, y );
}

void glutActiveMotion( int x, int y )
{   
    input->processActiveMotion( x, y );
}

gravManager::gravManager()
{
    windowWidth = 800; windowHeight = 600;
    holdCounter = 0;
    camX = 0.0f;
    camY = 0.0f;
    camZ = 9.0f;
}

gravManager::~gravManager()
{
    
}

void gravManager::draw()
{
    //audioSession_listener.printLevels();
    
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    glLoadIdentity();
    gluLookAt(camX, camY, camZ, 0.0, 0.0, -25.0, 0.0, 1.0, 0.0);
    
    earth->draw();
    
    GLUquadric* sphereQuad = gluNewQuadric();
    gluSphere( sphereQuad, audioSession_listener.getLevelAvg()*50.0f,
                200, 200 );
    
    // iterate through all sources and draw here
    // polygon offset to fix z-fighting of coplanar polygons (videos)
    //glEnable( GL_POLYGON_OFFSET_FILL );
    //glPolygonOffset( 0.1, 1.0 );
    
    //glPolygonMode( GL_FRONT, GL_FILL );
    //float pOffset = 0.1;
    
    std::vector<RectangleBase*>::const_iterator si;
    
    // draw line to geographical position
    for ( si = drawnObjects->begin(); si != drawnObjects->end(); si++ )
    {
        if ( !(*si)->isGrouped() )
        {
            drawCurvedEarthLine( (*si)->getLat(), (*si)->getLon(),
                                (*si)->getX(), (*si)->getY(), (*si)->getZ() );
        }
    }
    
    // this makes the depth buffer read-only for this bit - this prevents
    // z-fighting on the videos which are coplanar
    glDepthMask( GL_FALSE );
    
    //printf( "glutDisplay::drawing objects\n" );
    for ( si = drawnObjects->begin(); si != drawnObjects->end(); si++ )
    {
        // only draw if not grouped - groups are responsible for
        // drawing their members
        if ( !(*si)->isGrouped() )
        {
            if ( audioEnabled )
            {
                if ( (*si)->getSiteID().compare("") != 0 )
                {
                    float level;
                    level = audioSession_listener.getLevel( 
                                    (*si)->getSiteID() );
                    // -2.0f is our default value for not finding the level
                    if ( level > -1.999f )
                        (*si)->setEffectVal( (level*2.0f) );
                }
            }
            //printf( "glutDisplay::drawing object %s\n", (*si)->getName().c_str());
            (*si)->draw();
        }
        else
        {
            //printf( "%s is grouped, not drawing\n", (*si)->getName().c_str() );
            //printf( "its group is %s\n", (*si)->getGroup()->getName().c_str() );
        }
        //printf( "glutDisplay: siteID: %s level %f\n", 
        //        (*si)->getSiteID().c_str(),
        //        audioSession_listener.getLevel( (*si)->getSiteID().c_str() ) );
    }
    // back to writeable z-buffer for proper earth/line rendering
    glDepthMask( GL_TRUE );
    //printf( "glutDisplay::done drawing objects\n" );
    
    // draw the click-and-drag selection box
    if ( holdCounter > 1 && drawSelectionBox )
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // the main box
        glBegin(GL_QUADS);
        
        glColor4f( 0.1f, 0.2f, 1.0f, holdCounter/25.0f * 0.25f );
        
        glVertex3f(input->getDragStartX(), input->getDragStartY(), 0.0f);
        glVertex3f(input->getDragEndX(), input->getDragStartY(), 0.0f);
        glVertex3f(input->getDragEndX(), input->getDragEndY(), 0.0f);
        glVertex3f(input->getDragStartX(), input->getDragEndY(), 0.0f);
        
        glEnd();
        
        // the outline
        glBegin(GL_LINE_LOOP);
        
        glColor4f( 0.5f, 0.6f, 1.0f, holdCounter/25.0f * 0.25f );
        
        glVertex3f(input->getDragStartX(), input->getDragStartY(), 0.0f);
        glVertex3f(input->getDragEndX(), input->getDragStartY(), 0.0f);
        glVertex3f(input->getDragEndX(), input->getDragEndY(), 0.0f);
        glVertex3f(input->getDragStartX(), input->getDragEndY(), 0.0f);
        
        glEnd();
        
        glDisable(GL_BLEND);
    }
    
    //glDisable( GL_POLYGON_OFFSET_FILL );
    
    glutSwapBuffers();
    
    //printf( "holdcounter is %i\n", holdCounter );
    if ( !input->isLeftButtonHeld() && holdCounter > 0 )
        holdCounter-=2;
}

void gravManager::clearSelected()
{
    printf( "clearing\n" );
    for ( std::vector<RectangleBase*>::iterator sli = selectedObjects->begin();
            sli != selectedObjects->end(); sli++ )
        (*sli)->setSelect(false);
    selectedObjects->clear();
}

void gravManager::ungroupAll()
{
    printf( "deleting %i groups\n", siteIDGroups->size() );
    std::vector<RectangleBase*>::iterator it;
    for ( it = drawnObjects->begin(); it != drawnObjects->end(); )
    {
        Group* g = dynamic_cast<Group*>((*it));
        if ( g != NULL )
        {
            g->removeAll();
            it = drawnObjects->erase(it);
            
            if ( g->isSelected() )
            {
                std::vector<RectangleBase*>::iterator j =
                    selectedObjects->begin();
                while ( (*j) != g ) j++;
                selectedObjects->erase( j );
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
    siteIDGroups->clear();
    printf( "siteIDgroups cleared\n" );
}

void gravManager::retileVideos()
{
    x = -7.0f;
    y = 5.5f;
    float buffer = 1.0f; // space between videos
    
    std::vector<RectangleBase*>::iterator si;
    for ( si = drawnObjects->begin(); si != drawnObjects->end(); si++ )
    {
        if ( !(*si)->isGrouped() )
        {
            (*si)->move(x,y);
            std::vector<RectangleBase*>::iterator next = si + 1;
            if ( next == drawnObjects->end() ) next = drawnObjects->begin();
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

void gravManager::moveToTop( RectangleBase* object )
{
    std::vector<RectangleBase*>::iterator i = drawnObjects->begin();
    while ( (*i) != object ) i++;
    moveToTop( i );
}

void gravManager::moveToTop( std::vector<RectangleBase*>::iterator i )
{
    RectangleBase* temp = (*i);
    drawnObjects->erase( i );
    drawnObjects->push_back( temp );
    
    Group* g = dynamic_cast<Group*>( temp );
    if ( g != NULL )
    {
        for ( int i = 0; i < g->numObjects(); i++ )
            moveToTop( (*g)[i] );
    }
}

void gravManager::drawCurvedEarthLine( float lat, float lon,
                                float destx, float desty, float destz )
{
    // old method
    float sx, sy, sz;
    earth->convertLatLong( lat, lon, sx, sy, sz );
    float vecX = (sx - earth->getX()) * 0.1f;
    float vecY = (sy - earth->getY()) * 0.1f;
    float vecZ = (sz - earth->getZ()) * 0.1f;
    float tx = vecX + sx;
    float ty = vecY + sy;
    float tz = vecZ + sz;
    
    int iter = 10;
    
    glColor3f( 0.0f, 1.0f, 0.0f );
    glLineWidth( 2.0f );
    glBegin( GL_LINE_STRIP );
    glVertex3f( sx, sy, sz );
    for ( int i = 0; i < iter; i++ )
    {
        glVertex3f( tx, ty, tz );
        float weight = ((float)(iter-i))/(float)iter;
        weight *= weight;
        tx += (vecX  * weight) +
                ((destx-tx) * (1.0f-weight));
        ty += (vecY * weight) +
                ((desty-ty) * (1.0f-weight));
        tz += (vecZ * weight) +
                ((destz-tz) * (1.0f-weight));
    }
    //glVertex3f( tx, ty, tz );
    //glVertex3f( dx, dy, sz );
    glEnd();
        
    // new method
    /*
    float earthx, earthy, earthz;
    earth->convertLatLong( lat, lon, earthx, earthy, earthz );
    
    // this vector starts out as the one pointing into the lat/lon point,
    // along the line to the center of the earth
    float earthVecX = (earthx - earth->getX()) * -0.2f;
    float earthVecY = (earthy - earth->getY()) * -0.2f;
    float earthVecZ = (earthz - earth->getZ()) * -0.2f;
    
    // this assumes that the general direction between the destination points
    // and the earth is along the z-axis
    
    glColor3f( 0.0f, 1.0f, 0.0f );
    glLineWidth( 2.0f );
    
    float fx = earthx;
    float fy = earthy;
    float fz;
    
    // if the point isn't blocked by the earth, draw straight line
    
    // move the first target point towards the camera based on how far the
    // object is from the earth point in x/y
    float xydiff = (abs(destx-earthx)+abs(desty-earthy))/
                                            1.5f;
    printf( "xydiff: %f\n", xydiff );
    if ( earthz > earth->getZ() )
    {
        fz = earthz;
    }
    else
    {
        printf( "behind\n" );
        fz = earth->getZ() + abs((earthz-earth->getZ()));
    }
    fz = std::min( fz+xydiff, destz-1.0f );
    
    // shift the x/y out a bit on the edges
    fx += destx-earthx/7.0f;
    fy += desty-earthy/7.0f;
    
    // go out to the radius of the earth, then average the path
    // to travel progressively from -Z to the vector pointing directly to the
    // point on the earth

    float zDist = earthz - destz;
    int iter = ceil( abs(zDist/2.0f) );
    printf( "%i iterations\n", iter );
    //printf( "in averaging mode\n" );
    
    // find the radius point
    float rx, ry, rz;
    rz = earth->getZ();
    float xDist = earthx - earth->getX();
    float yDist = earthy - earth->getY();
    float curRadius = sqrt((xDist)*(xDist) +
                           (yDist)*(yDist));
    printf( "radius & curradius: %f, %f\n", earth->getRadius(), curRadius );
    rx = earth->getX() + (xDist*(earth->getRadius()*1.7f/curRadius));
    ry = earth->getY() + (yDist*(earth->getRadius()*1.7f/curRadius));
    printf( "radiuspoint: %f,%f,%f\n", rx,ry,rz );
    
    // draw & average the vectors
    float tx = destx, ty = desty, tz = destz;
    */  
    //glVertex3f( destx, desty, destz );
    /*printf( "earth x,y: %f,%f\n", earthx, earthy );
    printf( "fx,fy,fz: %f,%f,%f\n", fx, fy, fz );
    int numVecs = 4;
    int vecs[numVecs][3];
    vecs[0][0] = (fx-tx)/2.0f; vecs[0][1] = (fy-ty)/2.0f;
                vecs[0][2] = (fz-tz)/2.0f;
    vecs[1][0] = (rx-fx)/5.0f; vecs[1][1] = (ry-fy)/5.0f;
                vecs[1][2] = (rz-fz)/5.0f;
    vecs[2][0] = 0.0f;  vecs[2][1] = 0.0f;  vecs[2][2] = -0.5f;
    vecs[3][0] = earthVecX; vecs[3][1] = earthVecY;
                vecs[3][2] = earthVecZ;*/
    
    /*glBegin( GL_LINE_STRIP );
    for ( int n = 0; n < 1; n++ )
    {
        for ( int i = 0; i < iter; i++ )
        {
            glVertex3f( tx, ty, tz );
            float weight = ((float)(iter-i))/(float)iter;
            weight *= weight;
            tx += (vecs[n][0] * weight) +
                    (vecs[n+1][0] * (1.0f-weight));
            ty += (vecs[n][1] * weight) +
                    (vecs[n+1][1] * (1.0f-weight));
            tz += (vecs[n][2] * weight) +
                    (vecs[n+1][2] * (1.0f-weight));
        }
    }
    glEnd();*/
    
    /*
    // method: add the start vector, averaging in the pointing-in vector, then
    // if we get close to the earth, average in the pointing-out vector
    float distance = sqrt(((earthx-tx)*(earthx-tx))+
                         ((earthy-ty)*(earthy-ty))+
                         ((earthz-tz)*(earthz-tz)));
    //printf( "starting distance: %f\n", distance );
    float startVecX = (fx-destx)/7.0f;
    float startVecY = (fy-desty)/7.0f;
    float startVecZ = (fz-destz)/7.0f;
    float vecX = startVecX; float vecY = startVecY; float vecZ = startVecZ;
    int count = 0;
    
    glColor3f( 0.0f, 0.0f, 1.0f );
    glBegin(GL_LINE_STRIP);
    while ( count < iter )
    {
        glVertex3f( tx, ty, tz );
        tx += vecX; ty += vecY; tz += vecZ;
        float weight = ((float)(iter-count))/(float)iter;
        
        vecX = (vecX*weight)+(earthVecX*(1-weight));
        vecY = (vecY*weight)+(earthVecY*(1-weight));
        vecZ = (vecZ*weight)+(earthVecZ*(1-weight));
        //distance = sqrt(((earthx-tx)*(earthx-tx))+
        //               ((earthy-ty)*(earthy-ty))+
        //                ((earthz-tz)*(earthz-tz)));
        //printf( "distance: %f\n", distance );
        count++;
    }
    glVertex3f( earthx, earthy, earthz );
    glEnd();
    
    // test render of lines with no averaging
    glColor3f( 1.0f, 0.0f, 0.0f );
    glBegin(GL_LINE_STRIP);
    glVertex3f( destx, desty, destz );
    glVertex3f( fx, fy, fz );
    //glVertex3f( rx, ry, rz );
    //glVertex3f( earthx, earthy, earthz );
    glEnd();
    */
}

void gravManager::setBoxSelectDrawing( bool draw )
{
    drawSelectionBox = draw;
}

int gravManager::getWindowWidth()
{
    return windowWidth;
}

int gravManager::getWindowHeight()
{
    return windowHeight;
}

void gravManager::setWindowWidth( int w )
{
    windowWidth = w;
}

void gravManager::setWindowHeight( int h )
{
    windowHeight = h;
}

bool gravManager::usingSiteIDGroups()
{
    return enableSiteIDGroups;
}

void gravManager::setSiteIDGrouping( bool site )
{
    enableSiteIDGroups = site;
}

void gravManager::incrementHoldCounter()
{
    if ( holdCounter < 25 ) holdCounter++;
}

int gravManager::getHoldCounter()
{
    return holdCounter;
}

float gravManager::getCamX()
{
    return camX;
}

float gravManager::getCamY()
{
    return camY;
}

float gravManager::getCamZ()
{
    return camZ;
}

void gravManager::setCamX( float x )
{
    camX = x;
}

void gravManager::setCamY( float y )
{
    camY = y;
}

void gravManager::setCamZ( float z )
{
    camZ = z;
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
    sources->push_back( source );
    source->updateName();
    drawnObjects->push_back( source );
    
    // do some basic grid positions
    x += 6.0f;
    if ( x > 9.0f )
    {
        x = -7.5f;
        y -= 6.0f;
    }
    
    //printf( "size of sources is %i\n", sources->size() );

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
    for ( si = sources->begin(); si != sources->end(); si++ )
    {
        if ( (*si)->getssrc() == ssrc )
        {
            printf( "source is %s\n", (*si)->getName().c_str() );
            RectangleBase* temp = (RectangleBase*)(*si);
            std::vector<RectangleBase*>::iterator i = drawnObjects->begin();
            while ( (*i) != temp ) i++;
            drawnObjects->erase( i );
            
            if ( temp->isSelected() )
            {
                std::vector<RectangleBase*>::iterator j =
                    selectedObjects->begin();
                while ( (*j) != temp ) j++;
                selectedObjects->erase( j );
            }
            
            delete (*si);
            sources->erase( si );
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
    std::string dataS( data, data_len );
    //printf( "listener::RTCP_APP: %s,%s\n", appS.c_str(), dataS.c_str() );
    //printf( "listener::RTCP_APP: data length is %i\n", data_len );
    
    if ( appS.compare( "site" ) == 0 && grav->usingSiteIDGroups() )
    {
        // vic sends 4 nulls at the end of the rtcp_app string for some reason,
        // so chop those off
        dataS = std::string( dataS, 0, 32 );
        std::vector<VideoSource*>::iterator i = sources->begin();
        printf( "in rtcp app, got %i sources\n", sources->size() );
        
        // sometimes, if groups are enabled by default, we can get RTCP APP
        // before we get any sources added, resulting in a crash when we try
        // and dereference the sources pointer - so skip this if we don't have
        // any sources yet
        if ( sources->size() == 0 ) return;
        while ( (*i)->getssrc() != ssrc )
        {
            i++;
            
            if ( i == sources->end() ) return;
        }
        
        if ( !(*i)->isGrouped() )
        {
            Group* g;
            std::map<std::string,Group*>::iterator mapi =
                                            siteIDGroups->find(dataS);
            
            if ( mapi == siteIDGroups->end() )
            {
                g = new Group( 0.0f, 0.0f );
                g->setName( dataS );
                g->setSiteID( dataS );
                g->setTexture( borderTex, borderWidth, borderHeight );
                drawnObjects->push_back( g );
                siteIDGroups->insert( std::pair<std::string,Group*>(dataS, g) );
            }
            else
            {
                g = mapi->second;
            }
            
            (*i)->setSiteID( dataS );
            g->add( *i );
            grav->retileVideos();
        }
    }
}
