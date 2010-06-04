#include <VPMedia/video/VPMVideoDecoder.h>
#include <VPMedia/video/VPMVideoBufferSink.h>
#include <VPMedia/video/format.h>
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
#include "Runway.h"
#include "Earth.h"
#include "PNGLoader.h"
#include "InputHandler.h"
#include "VideoListener.h"
#include "TreeControl.h"
#include "LayoutManager.h"

#include "gravManager.h"

gravManager::gravManager()
{
    windowWidth = 0; windowHeight = 0; // this should be set immediately
                                       // after init
    holdCounter = 0; drawCounter = 0; autoCounter = 0;
    camX = 0.0f;
    camY = 0.0f;
    camZ = 9.0f;

    sources = new std::vector<VideoSource*>();
    drawnObjects = new std::vector<RectangleBase*>();
    selectedObjects = new std::vector<RectangleBase*>();
    siteIDGroups = new std::map<std::string,Group*>();

    sourcesToDelete = new std::vector<VideoSource*>();

    layouts = new LayoutManager();
    screenRect.setName( "screen rectangle" );
    screenRect.setAnimation( false );

    earthRect.setAnimation( false );
    earthRect.setPos( 0.0f, 0.0f );
    earthRect.setScale( 10.0f, 10.0f );

    runway = new Runway( -10.0f, 0.0f );
    runway->setScale( 2.0f, 10.0f );
    drawnObjects->push_back( runway );

    usingThreads = false;
    useRunway = true;

    audioEnabled = false;

    sourceCount = 0;

    sourceMutex = mutex_create();
}

gravManager::~gravManager()
{
    delete sources;
    delete drawnObjects;
    delete selectedObjects;
    delete siteIDGroups;

    delete layouts;

    delete runway;

    delete sourcesToDelete;

    mutex_free( sourceMutex );
}

void gravManager::draw()
{   
    //audioSession_listener.printLevels();
    
    // don't draw if either of these objects haven't been initialized yet
    if ( !earth || !input ) return;
    
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    glLoadIdentity();
    gluLookAt(camX, camY, camZ, 0.0, 0.0, -25.0, 0.0, 1.0, 0.0);
    
    //GLUquadric* sphereQuad = gluNewQuadric();
    //gluSphere( sphereQuad, audioSession_listener->getLevelAvg()*50.0f,
    //            200, 200 );
    
    // test text drawing
    /*glPushMatrix();
    glColor4f( 1.0f, 1.0f, 1.0f, 0.5f );
    glScalef( 0.05f, 0.05f, 0.05f );
    glTranslatef( 0.0f, 0.0f, 0.0f );
    glRasterPos2f( 0.0f, 0.0f );
    const char* text = "TEST TEXT";
    FTFont* font = new FTBufferFont("/usr/share/fonts/truetype/freefont/FreeSans.ttf");
    font->FaceSize(100);
    font->Render( text );
    glPopMatrix();*/
    
    // set it to update names only every 30 frames
    bool updateNames = false;
    if ( drawCounter > 29 )
    {
        updateNames = true;
        drawCounter = 0;
    }
    
    // polygon offset to fix z-fighting of coplanar polygons (videos)
    // disabled, since making the depth buffer read-only in some area takes
    // care of this issue
    //glEnable( GL_POLYGON_OFFSET_FILL );
    //glPolygonOffset( 0.1, 1.0 );
    
    //glPolygonMode( GL_FRONT, GL_FILL );
    //float pOffset = 0.1;
    
    std::vector<RectangleBase*>::const_iterator si;
    
    lockSources();

    // periodically automatically rearrange if on automatic - take last object
    // and put it in center
    if ( autoCounter == 0 && getMovableObjects().size() > 0 && !useRunway )
    {
        std::vector<RectangleBase*> outerObjs = getMovableObjects();
        std::vector<RectangleBase*> innerObj( outerObjs.begin(),
                                                outerObjs.begin()+1 );
        outerObjs.erase( outerObjs.begin() );

        layouts->focus( getScreenRect(), outerObjs, innerObj );

        moveToTop( innerObj[0] );
    }

    // delete sources that need to be deleted - see deleteSource for the reason
    if ( sourcesToDelete->size() > 0 )
    {
        for ( unsigned int i = 0; i < sourcesToDelete->size(); i++ )
        {
            delete (*sourcesToDelete)[i];
        }
        sourcesToDelete->clear();
    }

    // draw line to geographical position, selected ones on top (and bigger)
    for ( si = drawnObjects->begin(); si != drawnObjects->end(); si++ )
    {
        RGBAColor col = (*si)->getColor();
        glColor4f( col.R, col.G, col.B, col.A );
        if ( !(*si)->isGrouped() && !(*si)->isSelected() )
        {
            //drawCurvedEarthLine( (*si)->getLat(), (*si)->getLon(),
            //                    (*si)->getX(), (*si)->getY(), (*si)->getZ() );
            drawEarthPoint( (*si)->getLat(), (*si)->getLon(), 3.0f );
        }
    }
    for ( si = selectedObjects->begin(); si != selectedObjects->end(); si++ )
    {
        RGBAColor col = (*si)->getColor();
        glColor4f( col.R, col.G, col.B, col.A );
        if ( !(*si)->isGrouped() )
        {
            //drawCurvedEarthLine( (*si)->getLat(), (*si)->getLon(),
            //                    (*si)->getX(), (*si)->getY(), (*si)->getZ() );
            drawEarthPoint( (*si)->getLat(), (*si)->getLon(), 6.0f );
        }
    }
    
    earth->draw();
    
    // this makes the depth buffer read-only for this bit - this prevents
    // z-fighting on the videos which are coplanar
    glDepthMask( GL_FALSE );
    
    //printf( "glutDisplay::drawing objects\n" );
    // iterate through all objects to be drawn, and draw
    for ( si = drawnObjects->begin(); si != drawnObjects->end(); si++ )
    {
        // do things we only want to do every X frames,
        // like updating the name
        if ( updateNames )
        {
            // only bother updating it on the tree if it actually
            // changes - to suppress "" from getting shown
            if ( (*si)->updateName() )
                tree->updateObjectName( (*si) );
        }
        
        // only draw if not grouped - groups are responsible for
        // drawing their members
        if ( !(*si)->isGrouped() )
        {
            // set the audio effect level
            if ( audioEnabled )
            {
                // TODO: do a lookup with CNAME if siteIDs aren't enabled
                if ( (*si)->getSiteID().compare("") != 0 )
                {
                    float level;
                    level = audio->getLevel( (*si)->getSiteID() );
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
    
    unlockSources();

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
    
    glFlush();
    
    //glDisable( GL_POLYGON_OFFSET_FILL );
    
    //glutSwapBuffers(); // TODO: change this with wx's buffer-swapping? done?
    // works in glcanvas's draw?
    
    if ( !input->isLeftButtonHeld() && holdCounter > 0 )
        holdCounter-=2;
    else
        incrementHoldCounter();
    drawCounter++;
    autoCounter = (autoCounter+1)%900;
}

void gravManager::clearSelected()
{
    for ( std::vector<RectangleBase*>::iterator sli = selectedObjects->begin();
            sli != selectedObjects->end(); ++sli )
        (*sli)->setSelect(false);
    selectedObjects->clear();
}

void gravManager::ungroupAll()
{
    lockSources();

    printf( "deleting %i groups\n", siteIDGroups->size() );
    std::map<std::string,Group*>::iterator it;
    for ( it = siteIDGroups->begin(); it != siteIDGroups->end(); ++it )
    {
        Group* g = (*it).second;
        if ( g != NULL )
        {
            g->removeAll();
            removeFromLists( g );
            delete g;
            
            printf( "single group deleted\n" );
        }
    }
    siteIDGroups->clear();
    printf( "siteIDgroups cleared\n" );

    unlockSources();
}

void gravManager::addTestObject()
{
    RectangleBase* obj = new RectangleBase( 0.0f, 0.0f );
    drawnObjects->push_back( obj );
    obj->setName( "TEST" );
    //runway->add( obj );
    obj->setTexture( borderTex, borderWidth, borderHeight );
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
    float vecX = (sx - earth->getX()) * 0.08f;
    float vecY = (sy - earth->getY()) * 0.08f;
    float vecZ = (sz - earth->getZ()) * 0.08f;
    float tx = vecX + sx;
    float ty = vecY + sy;
    float tz = vecZ + sz;
    
    int iter = 15;
    float zdist = destz-tz;
    float maxzdist = (earth->getZ()+earth->getRadius())-destz;
    float distanceScale = fabs(zdist/maxzdist);
    int i = 0;
    
    glLineWidth( 2.0f );
    glBegin( GL_LINE_STRIP );
    glVertex3f( sx, sy, sz );
    
    while ( zdist > 1.0f )
    {
        glVertex3f( tx, ty, tz );
        float weight = (((float)(iter-i))/(float)iter)*0.6f;
        //float weight = 0.2f;
        //weight *= weight;
        float xpush = 0.0f;
        float ypush = 0.0f;
        //float curDistX = fabs(tx-earth->getX());
        //float curDistY = fabs(ty-earth->getY());
        
        // this will push out the current point on the line away from the
        // earth so it doesn't clip, scaled based on how far we are from the
        // destination
        xpush = 1.5f * distanceScale;
        ypush = 1.5f * distanceScale;
        if ( tx < earth->getX() ) xpush *= -1.0f;
        if ( ty < earth->getY() ) ypush *= -1.0f;
        
        // move the current point forward, based on progressively averaging
        // the earth-pointing-out vector with the vector pointing towards
        // the destination point
        tx += (vecX * weight) +
                ((destx-tx) * (1.0f-weight))
                + xpush;
        ty += (vecY * weight) +
                ((desty-ty)  * (1.0f-weight))
                + ypush;
        tz += (vecZ * weight) +
                ((destz-tz) * (1.0f-weight));
                
        zdist = destz-tz;
        distanceScale = fabs(zdist/maxzdist);
        i++;
    }
    // shift the z back a bit so it doesn't overshoot the object
    if ( tz > destz - 0.3f ) tz -= 0.3f;
    glVertex3f( tx, ty, tz );
    // -0.05f is so the line doesn't poke through the objects
    glVertex3f( destx, desty, destz-0.05f );
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

void gravManager::drawEarthPoint( float lat, float lon, float size )
{
    float sx, sy, sz;
    earth->convertLatLong( lat, lon, sx, sy, sz );

    glPointSize( size );
    glBegin( GL_POINTS );
    glVertex3f( sx, sy, sz );
    glEnd();
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

void gravManager::setWindowSize( int w, int h )
{
    windowWidth = w;
    windowHeight = h;
    GLdouble screenL, screenR, screenU, screenD;
    
    GLUtil* glUtil = GLUtil::getInstance();
    GLdouble dummy; // for Z which we don't need
    // update the screen size (in world space) coordinates
    glUtil->screenToWorld( (GLdouble)windowWidth, (GLdouble)windowHeight,
                          (GLdouble)0.990991f, &screenR, &screenU, &dummy );
    glUtil->screenToWorld( (GLdouble)0.0f, (GLdouble)0.0f,
                          (GLdouble)0.990991f, &screenL, &screenD, &dummy );
    
    screenRect.setPos( (screenL+screenR)/2.0f, (screenU+screenD)/2.0f);
    screenRect.setScale( screenR-screenL, screenU-screenD );

    // resize the runway to go on the left side
    // TODO: make this work for horizontal, vertical, let user change it
    runway->setScale( fabs( screenR - screenL ) * 0.07f,
                      fabs( screenU - screenD ) * 0.93f, true );
    runway->setPos( screenL + (fabs( screenU - screenD ) * 0.035f) +
                  runway->getDestWidth() / 2.0f, ( screenU + screenD ) / 2.0f );
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

void gravManager::scaleSelectedObjects( float scaleAmt )
{
    for ( unsigned int i = 0; i < selectedObjects->size(); i++ )
    {
        RectangleBase* temp = (*selectedObjects)[i];
        temp->setScale( temp->getScaleX()+temp->getScaleX()*scaleAmt,
                         temp->getScaleY()+temp->getScaleY()*scaleAmt );
    }
}

std::vector<VideoSource*>* gravManager::getSources()
{
    return sources;
}

std::vector<RectangleBase*>* gravManager::getDrawnObjects()
{
    return drawnObjects;
}

std::vector<RectangleBase*>* gravManager::getSelectedObjects()
{
    return selectedObjects;
}

std::map<std::string,Group*>* gravManager::getSiteIDGroups()
{
    return siteIDGroups;
}

std::vector<RectangleBase*> gravManager::getMovableObjects()
{
    std::vector<RectangleBase*> objects;
    for ( unsigned int i = 0; i < drawnObjects->size(); i++ )
    {
        if ( !(*drawnObjects)[i]->isGrouped() &&
                (*drawnObjects)[i]->isSelectable() )
        {
            objects.push_back( (*drawnObjects)[i] );
        }
    }
    return objects;
}

std::vector<RectangleBase*> gravManager::getUnselectedObjects()
{
    std::vector<RectangleBase*> unselectedObjects;
    std::vector<RectangleBase*> movableObjs = getMovableObjects();
    for ( unsigned int i = 0; i < movableObjs.size(); i++ )
    {
        if ( !movableObjs[i]->isSelected() )
        {
            unselectedObjects.push_back( movableObjs[i] );
        }
    }

    return unselectedObjects;
}

void gravManager::addNewSource( VideoSource* s )
{
    if ( s == NULL ) return;

    s->setTexture( borderTex, borderWidth, borderHeight );

    lockSources();

    sources->push_back( s );
    drawnObjects->push_back( s );
    s->updateName();

    sourceCount++;

    if ( tree != NULL )
        tree->addObject( (RectangleBase*)s );

    if ( useRunway && sourceCount > 9 )
        runway->add( s );
    else if ( !useRunway )
    {
        std::vector<RectangleBase*> outerObjs = getMovableObjects();
        std::vector<RectangleBase*> innerObj( outerObjs.end()-1,
                                                outerObjs.end() );
        outerObjs.erase( outerObjs.end()-1 );

        layouts->focus( getScreenRect(), outerObjs, innerObj );
    }

    unlockSources();
}

void gravManager::deleteSource( std::vector<VideoSource*>::iterator si )
{
    RectangleBase* temp = (RectangleBase*)(*si);
    VideoSource* s = (*si);
    
    lockSources();
    
    removeFromLists( temp );

    sources->erase( si );

    // TODO need case for runway grouping
    if ( temp->isGrouped() )
    {
        Group* g = temp->getGroup();
        // delete the group the object was in if this is the last object in it
        // and it's an automatically mad siteID group
        // TODO probably have a better metric for determining auto-siteID groups
        if ( g->getSiteID().compare( "" ) != 0 && g->numObjects() == 1 )
        {
            // note this duplicates the deleteGroup function since that does
            // mutex locking itself, and we already did that here
            removeFromLists( (RectangleBase*)g );

            // remove the group from the list of siteIDgroups
            std::map<std::string,Group*>::iterator gi =
                    siteIDGroups->find( g->getSiteID() );
            siteIDGroups->erase( gi );

            delete g;
        }
        // if not a siteID group then remove the object from it. this should
        // work for runways, but should be more generic, ie for groups of
        // groups? maybe in removefromlists, but careful not to degroup object
        // before it hits that siteID check above or siteIDgroups will have
        // invalid references
        else
        {
            g->remove( (*si) );
        }
    }

    // we need to do videosource's delete somewhere else, since this function
    // might be on a second thread, which would crash since the videosource
    // delete needs to do a GL call to delete its texture and GL calls can only
    // be on the main thread
    sourcesToDelete->push_back( s );

    unlockSources();
}

void gravManager::deleteGroup( Group* g )
{
    lockSources();

    removeFromLists( g );
    delete g;

    unlockSources();
}

void gravManager::removeFromLists( RectangleBase* obj )
{
    // remove it from the tree
    tree->removeObject( obj );

    // remove it from drawnobjects, if it is being drawn
    std::vector<RectangleBase*>::iterator i = drawnObjects->begin();
    while ( (*i) != obj ) i++;

    // TODO: confirm this (checking whether it actually is in drawnobjects
    //                      or not)
    if ( i != drawnObjects->end() )
        drawnObjects->erase( i );

    if ( obj->isSelected() )
    {
        std::vector<RectangleBase*>::iterator j =
            selectedObjects->begin();
        while ( (*j) != obj ) j++;
        selectedObjects->erase( j );
    }
}

void gravManager::setBorderTex( std::string border )
{
    borderTex = PNGLoader::loadPNG( "border.png",
                                    borderWidth, borderHeight );
}

Group* gravManager::createSiteIDGroup( std::string data )
{
    printf( "creating siteIDGroup\n" );

    Group* g = new Group( 0.0f, 0.0f );
    g->setName( data );
    g->setSiteID( data );
    g->setTexture( borderTex, borderWidth, borderHeight );

    // something that calls this function should mutex around it itself
    //lockSources();

    drawnObjects->push_back( g );
    siteIDGroups->insert( std::pair<std::string,Group*>( data, g ) );

    if ( tree != NULL )
        tree->addObject( g );

    /*if ( useRunway )
        runway->add( g );
    else
        retileVideos();*/

    //unlockSources();

    return g;
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

RectangleBase gravManager::getScreenRect()
{
    return screenRect;
}

RectangleBase gravManager::getEarthRect()
{
    return earthRect;
}

void gravManager::setEarth( Earth* e )
{
    earth = e;
}

void gravManager::setInput( InputHandler* i )
{
    input = i;
}

void gravManager::setTree( TreeControl* t )
{
    tree = t;
}

void gravManager::setAudio( AudioManager* a )
{
    if ( a != NULL )
        audioEnabled = true;
    else
        audioEnabled = false;

    audio = a;
}

TreeControl* gravManager::getTree()
{
    return tree;
}

void gravManager::lockSources()
{
    if ( usingThreads )
        mutex_lock( sourceMutex );
}

void gravManager::unlockSources()
{
    if ( usingThreads )
        mutex_unlock( sourceMutex );
}

void gravManager::setThreads( bool threads )
{
    usingThreads = threads;
}

bool gravManager::usingRunway()
{
    return useRunway;
}

void gravManager::setRunwayUsage( bool run )
{
    useRunway = run;

    std::vector<RectangleBase*>::iterator it = drawnObjects->begin();
    while ( (*it) != runway && it != drawnObjects->end() ) ++it;
    bool found = ( it != drawnObjects->end() );

    // make sure it's drawn if enabled, not drawn if not
    if ( useRunway && !found )
    {
        drawnObjects->push_back( runway );
    }
    else if ( !useRunway && found )
    {
        drawnObjects->erase( it );
    }
}
