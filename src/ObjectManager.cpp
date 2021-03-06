/*
 * @file ObjectManager.cpp
 *
 * Implementation of major grav functions - holds lists of drawn objects and
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
#include "gravUtil.h"
#include "RectangleBase.h"
#include "Group.h"
#include "Runway.h"
#include "Earth.h"
#include "InputHandler.h"
#include "VideoListener.h"
#include "TreeControl.h"
#include "LayoutManager.h"
#include "VenueClientController.h"
#include "SessionManager.h"
#include "SessionEntry.h"
#include "Camera.h"
#include "Point.h"

#include "ObjectManager.h"

#include <VPMedia/random_helper.h>

ObjectManager::ObjectManager()
{
    windowWidth = 0; windowHeight = 0; // this should be set immediately
                                       // after init
    holdCounter = 0;
    drawCounter = 0;
    autoCounter = 0;
    intersectCounter = 0;

    Point camPoint = Point( 0.0f, 0.0f, 9.0f );
    Point lookat( 0.0f, 0.0f, -25.0f );
    cam = new Camera( camPoint, lookat );

    sources = new std::vector<VideoSource*>();
    drawnObjects = new std::vector<RectangleBase*>();
    selectedObjects = new std::vector<RectangleBase*>();
    siteIDGroups = new std::map<std::string,Group*>();

    objectsToDelete = new std::vector<RectangleBase*>();
    objectsToAddToTree = new std::vector<RectangleBase*>();
    objectsToRemoveFromTree = new std::vector<RectangleBase*>();

    layouts = new LayoutManager();

    screenRectFull.setName( "screen rectangle full" );
    screenRectFull.setAnimation( false );
    RGBAColor fullColor;
    fullColor.R = 0.7f;
    fullColor.G = 0.7f;
    fullColor.B = 0.2f,
    fullColor.A = 0.6f;
    screenRectFull.setColor( fullColor );
    screenRectFull.setBorderScale( 0.0f );

    screenRectSub.setName( "screen rectangle sub" );
    screenRectSub.setAnimation( false );
    RGBAColor subColor;
    subColor.R = 0.2f;
    subColor.G = 0.7f;
    subColor.B = 0.6f,
    subColor.A = 0.6f;
    screenRectSub.setColor( subColor );
    screenRectSub.setBorderScale( 0.0f );

    earthRect.setAnimation( false );
    earthRect.setPos( 0.0f, 0.0f );
    earthRect.setScale( 10.0f, 10.0f );

    runway = new Runway( -10.0f, 0.0f );
    runway->setScale( 2.0f, 10.0f );
    drawnObjects->push_back( runway );

    headerString = "";
    useHeader = false;
    textScale = 0.01;
    textOffset = 0.25;

    usingThreads = false;
    useRunway = true;
    gridAuto = false;
    autoFocusRotate = false;
    sessionFocus = false;
    focusSession = "";

    audioEnabled = false;
    audioFocusTrigger = false;
    audio = NULL;

    orbiting = false;

    sourceMutex = mutex_create();
    lockCount = 0;

    graphicsDebugView = false;
    pixelCount = 0;

    venueClientController = NULL; // just for before it gets set
}

ObjectManager::~ObjectManager()
{
    ungroupSiteIDGroups();

    doDelayedDelete();

    delete sources;
    delete drawnObjects;
    delete selectedObjects;
    delete siteIDGroups;

    delete layouts;

    delete runway;

    delete cam;

    delete objectsToDelete;
    delete objectsToAddToTree;
    delete objectsToRemoveFromTree;

    mutex_free( sourceMutex );
}

void ObjectManager::draw()
{
    // don't draw if either of these objects haven't been initialized yet
    if ( !earth || !input ) return;

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    cam->animateValues();
    cam->doGLLookat();

    // audio test drawing
    /*if ( audioAvailable() )
    {
        GLUquadric* sphereQuad = gluNewQuadric();
        if ( true ) //drawCounter % 2 == 0 )
        {
            float avg = audio->getLevelAvg();
            if ( avg > 0.0001f )
                overalllevel = avg;
        }
        gluSphere( sphereQuad, overalllevel * 30.0f + 0.5f, 50, 50 );
    }*/

    // set it to update names only every 30 frames
    bool updateNames = false;
    if ( drawCounter == 0 )
    {
        updateNames = true;
        if ( audioAvailable() )
            audio->updateNames();
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
    if ( !orbiting && autoCounter == 0 && getMovableObjects().size() > 0 &&
            autoFocusRotate )
    {
        outerObjs = getMovableObjects();
        innerObjs = std::vector<RectangleBase*>( outerObjs.begin(),
                                                    outerObjs.begin()+1 );
        outerObjs.erase( outerObjs.begin() );

        std::map<std::string, std::vector<RectangleBase*> > data = \
            std::map<std::string, std::vector<RectangleBase*> >();
        data["inners"] = innerObjs;
        data["outers"] = outerObjs;
        layouts->arrange( "aspectFocus", getScreenRect(), RectangleBase(),
                            data );

        moveToTop( innerObjs[0] );

        outerObjs.clear();
        innerObjs.clear();
    }

    // add objects to tree that need to be added - similar to delete, tree is
    // modified on the main thread (in other WX places) so
    if ( objectsToAddToTree->size() > 0 && tree != NULL )
    {
        for ( unsigned int i = 0; i < objectsToAddToTree->size(); i++ )
        {
            tree->addObject( (*objectsToAddToTree)[i] );
        }
        objectsToAddToTree->clear();

        // bit of a hack to force orbit on adding a new video
        // can't do it in addsource since GL stuff can only be on main thread
        if ( orbiting )
        {
            orbitVideos();
        }
    }
    // same for remove
    if ( objectsToRemoveFromTree->size() > 0 && tree != NULL )
    {
        for ( unsigned int i = 0; i < objectsToRemoveFromTree->size(); i++ )
        {
            tree->removeObject( (*objectsToRemoveFromTree)[i] );
        }
        objectsToRemoveFromTree->clear();
    }
    // delete sources that need to be deleted - see deleteSource for the reason
    doDelayedDelete();

    // draw point on geographical position, selected ones on top (and bigger)
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
    if ( !orbiting )
    {
        glDepthMask( GL_FALSE );
    }

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
            {
                if ( tree )
                {
                    tree->updateObjectName( (*si) );
                }

                VideoSource* vid = dynamic_cast<VideoSource*>( (*si) );
                if ( vid )
                {
                    std::map<std::string, std::string>::iterator mi =
                            thumbnailMap.find( vid->getName() );
                    if ( mi != thumbnailMap.end() )
                    {
                        vid->setAltAddress( mi->second );
                        gravUtil::logVerbose( "ObjectManager::draw(): %s alt "
                                "address set to %s\n", vid->getName().c_str(),
                                ( mi->second ).c_str() );
                    }
                }
            }
        }

        // only draw if not grouped - groups are responsible for
        // drawing their members
        if ( !(*si)->isGrouped() )
        {
            // set the audio effect level on the drawcounter, if audio is
            // enabled, and if it's selectable (excludes runway)
            // TODO maybe change this if meaning of selectable changes
            if ( audioAvailable() && drawCounter == 0 && (*si)->isSelectable() )
            {
                // had a really bizarre bug here - if uninitialized, would hit
                // > 0.01f check and succeed later if object was selected. what?
                float level = 0.0f;
                // if source has siteID, send that (and tell audiomanager to
                // only check siteIDs), if not, use cname if available
                if ( (*si)->getSiteID().compare("") != 0 )
                {
                    level = audio->getLevel( (*si)->getSiteID(), true, false );
                }
                else if ( (*si)->getAltName().compare("") != 0 )
                {
                    level = audio->getLevel( (*si)->getAltName(), true, true );
                }

                if ( level > 0.01f )
                {
                    innerObjs.push_back( (*si) );
                    audioFocusTrigger = true;
                }
                else
                {
                    outerObjs.push_back( (*si) );
                }
            }
            (*si)->draw();
        }
        else
        {
            // object is grouped, so group will draw it itself
        }
    }

    // do the audio focus if it triggered
    if ( audioAvailable() )
    {
        if ( audioFocusTrigger )
        {
            if ( !orbiting )
            {
                std::map<std::string, std::vector<RectangleBase*> > data =
                    std::map<std::string, std::vector<RectangleBase*> >();
                data["inners"] = innerObjs;
                data["outers"] = outerObjs;
                layouts->arrange( "aspectFocus", getScreenRect(),
                                    RectangleBase(), data );
                audioFocusTrigger = false;
            }
            else
            {
                // rotate earth to talking site here?
            }
        }

        outerObjs.clear();
        innerObjs.clear();
    }

    // check runway members for potential removal if outside
    if ( intersectCounter == 0 && runway->numObjects() > 0 &&
            runway->getColor().A > 0.01f )
        runway->handleOutsideMembers();

    if ( input->haveValidMousePos() )
    {
        // make an exception here to do intersect including border
        float border = sessionManager->getBorderSize();
        Bounds bounds = sessionManager->getBounds();
        bool sessionMouseover =
                   input->getMouseX() > bounds.L - border &&
                   input->getMouseX() < bounds.R + border &&
                   input->getMouseY() < bounds.U + border &&
                   input->getMouseY() > bounds.D - border;
        if ( sessionMouseover )
        {
            // keep sessionmanager on top, including if new videos come in
            moveToTop( sessionManager );
            if ( !sessionManager->isShown() )
            {
                sessionManager->show( true );
            }
        }
        else if ( !sessionMouseover && sessionManager->isShown() )
        {
            sessionManager->show( false );
        }
    }

    unlockSources();

    // check session manager for moved session entry objects & shift if
    // necessary - this needs to be outside the lock() since a shift might
    // trigger a session disable, which may delete a video which needs to lock
    // on its own (ie, we're not doing reentrant mutexes)
    if ( intersectCounter == 0 && sessionManager->isShown() )
        sessionManager->checkGUISessionShift();

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

    // header text drawing
    if ( useHeader )
    {
        glPushMatrix();

        glEnable( GL_BLEND );
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f( 0.953f, 0.431f, 0.129f, 0.5f );
        Bounds screenBounds = screenRectFull.getBounds();
        float textXPos = screenBounds.L + textOffset;
        float textYPos = screenBounds.U -
                ( ( headerTextBox.Upper().Yf() - headerTextBox.Lower().Yf() )
                        * textScale ) - textOffset;
        glTranslatef( textXPos, textYPos, 0.0f );
        glScalef( textScale, textScale, textScale );
        const char* text = headerString.c_str();
        GLUtil::getInstance()->getMainFont()->Render( text );
        glDisable( GL_BLEND );

        glPopMatrix();
    }

    // graphics debug drawing
    if ( graphicsDebugView )
    {
        glPushMatrix();

        GLCanvas* canvas = GLUtil::getInstance()->getCanvas();

        long drawTime = canvas->getDrawTime();
        float color = (33.0f - (float)drawTime) / 17.0f;
        glColor4f( 1.0f, color, color, 0.8f );
        Bounds screenBounds = screenRectFull.getBounds();
        glTranslatef( 0.0f, screenBounds.U * 0.9f, 0.0f );
        float debugScale = textScale / 2.5f;
        glScalef( debugScale, debugScale, debugScale );
        char text[100];
        sprintf( text,
                "Draw time: %3ld  Non-draw time: %3ld  Pixel count: %8ld "
                "FPS: %2.2f",
                canvas->getDrawTime(), canvas->getNonDrawTime(),
                videoListener->getPixelCount(), canvas->getFPS() );
        GLUtil::getInstance()->getMainFont()->Render( text );

        glPopMatrix();
    }

    // back to writeable z-buffer for proper earth/line rendering
    if ( !orbiting )
    {
        glDepthMask( GL_TRUE );
    }

    glFlush();

    // hold counter is a bit different than the others since alpha values
    // directly depend on it, as above
    if ( !input->isLeftButtonHeld() && holdCounter > 0 )
        holdCounter-=2;
    else if ( holdCounter < 24 )
        holdCounter++;

    drawCounter = ( drawCounter + 1 ) % 30;
    intersectCounter = ( intersectCounter + 1 ) % 20;
    autoCounter = ( autoCounter + 1 ) % 900;
}

void ObjectManager::clearSelected()
{
    for ( std::vector<RectangleBase*>::iterator sli = selectedObjects->begin();
            sli != selectedObjects->end(); ++sli )
        (*sli)->setSelect(false);
    selectedObjects->clear();
}

void ObjectManager::ungroupSiteIDGroups()
{
    lockSources();

    gravUtil::logVerbose( "ObjectManager::ungroupAll: deleting %i groups\n",
            siteIDGroups->size() );
    std::map<std::string,Group*>::iterator it;
    for ( it = siteIDGroups->begin(); it != siteIDGroups->end(); ++it )
    {
        Group* g = (*it).second;
        if ( g != NULL )
        {
            // even though group will remove its children in its destructor,
            // we should do it here first to make sure the children get readded
            // to the tree properly when the group is removed from the tree
            g->removeAll();
            removeFromLists( g );
            objectsToDelete->push_back( g );

            gravUtil::logVerbose( "ObjectManager::ungroupAll: group %s set to "
                    "be deleted on draw\n", g->getName().c_str() );
        }
    }
    siteIDGroups->clear();
    gravUtil::logVerbose( "ObjectManager::ungroupAll: siteIDgroups cleared\n" );

    unlockSources();
}

void ObjectManager::addTestObject()
{
    lockSources();

    RectangleBase* obj = new RectangleBase( 0.0f, 0.0f );
    drawnObjects->push_back( obj );
    bool useRandName = false;
    if ( useRandName )
    {
        int nameLength = 10;
        std::string randName;
        for( int i = 0; i < nameLength; i++ )
        {
            int rand = ((float)random32() / (float)random32_max() * 95) + 32;
            randName += (char)rand;
        }
        obj->setName( randName );
    }
    else if ( drawnObjects->size() % 2 == 0 )
    {
        obj->setName( " " );
    }
    else
    {
        obj->setName( "TEST" );
    }
    Texture t = GLUtil::getInstance()->getTexture( "border" );
    obj->setTexture( t.ID, t.width, t.height );
    obj->setUserDeletable( true );

    unlockSources();
}

void ObjectManager::tryDeleteObject( RectangleBase* obj )
{
    lockSources();

    // note this will only check userdeletable objects. Videos should probably
    // not be deletable.
    if ( obj->isUserDeletable() )
    {
        // false as second arg to not try to remove it from the tree
        // TODO fix this maybe, doesn't necessarily fit - only makes sense for
        // test object case
        removeFromLists( obj, false );
        delete obj;
    }

    unlockSources();
}

/*
 * Note this is NOT thread-safe, lockSources() should be called around any calls
 * to this.
 */
void ObjectManager::moveToTop( RectangleBase* object, bool checkGrouping )
{
    if ( object == NULL )
    {
        gravUtil::logError( "ObjectManager::moveToTop: object %s is NULL\n",
                            object->getName().c_str() );
        return;
    }

    std::vector<RectangleBase*>::iterator i = drawnObjects->begin();
    while ( i != drawnObjects->end() && (*i) != object ) ++i;
    if ( i == drawnObjects->end() )
    {
        gravUtil::logError( "ObjectManager::moveToTop: object %s not found in "
                            "draw list\n", object->getName().c_str() );
        return;
    }
    moveToTop( i, checkGrouping );
}

/*
 * Note this is NOT thread-safe, lockSources() should be called around any calls
 * to this.
 */
void ObjectManager::moveToTop( std::vector<RectangleBase*>::iterator i,
                                bool checkGrouping )
{
    RectangleBase* temp = (*i);
    RectangleBase* orig = temp;

    // find highest group in the chain, to move up group members from the top
    // of the chain
    while ( checkGrouping && temp->isGrouped() )
        temp = temp->getGroup();

    // do this to properly find iterator position, since i != temp now
    if ( temp != orig )
        moveToTop( temp, checkGrouping );
    else
    {
        drawnObjects->erase( i );
        drawnObjects->push_back( temp );

        if ( temp->isGroup() )
        {
            Group* g = (Group*)temp;
            for ( int i = 0; i < g->numObjects(); i++ )
                moveToTop( (*g)[i], false );
        }
    }
}

void ObjectManager::drawCurvedEarthLine( float lat, float lon,
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
    gravUtil::logVerbose( "xydiff: %f\n", xydiff );
    if ( earthz > earth->getZ() )
    {
        fz = earthz;
    }
    else
    {
        gravUtil::logVerbose( "behind\n" );
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
    gravUtil::logVerbose( "%i iterations\n", iter );
    //gravUtil::logVerbose( "in averaging mode\n" );

    // find the radius point
    float rx, ry, rz;
    rz = earth->getZ();
    float xDist = earthx - earth->getX();
    float yDist = earthy - earth->getY();
    float curRadius = sqrt((xDist)*(xDist) +
                           (yDist)*(yDist));
    gravUtil::logVerbose( "radius & curradius: %f, %f\n", earth->getRadius(),
            curRadius );
    rx = earth->getX() + (xDist*(earth->getRadius()*1.7f/curRadius));
    ry = earth->getY() + (yDist*(earth->getRadius()*1.7f/curRadius));
    gravUtil::logVerbose( "radiuspoint: %f,%f,%f\n", rx,ry,rz );

    // draw & average the vectors
    float tx = destx, ty = desty, tz = destz;
    */
    //glVertex3f( destx, desty, destz );
    /*gravUtil::logVerbose( "earth x,y: %f,%f\n", earthx, earthy );
    gravUtil::logVerbose( "fx,fy,fz: %f,%f,%f\n", fx, fy, fz );
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
    //gravUtil::logVerbose( "starting distance: %f\n", distance );
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
        //gravUtil::logVerbose( "distance: %f\n", distance );
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

void ObjectManager::drawEarthPoint( float lat, float lon, float size )
{
    float sx, sy, sz;
    earth->convertLatLong( lat, lon, sx, sy, sz );

    glPointSize( size );
    glBegin( GL_POINTS );
    glVertex3f( sx, sy, sz );
    glEnd();
}

void ObjectManager::setBoxSelectDrawing( bool draw )
{
    drawSelectionBox = draw;
}

int ObjectManager::getWindowWidth()
{
    return windowWidth;
}

int ObjectManager::getWindowHeight()
{
    return windowHeight;
}

void ObjectManager::setWindowSize( int w, int h )
{
    windowWidth = w;
    windowHeight = h;
    GLdouble screenL, screenR, screenU, screenD;
    GLUtil* glUtil = GLUtil::getInstance();

    // save original camera position & reset in order to project from correct
    // spot and find the rectangle relative/facing to the original camera
    // position
    Point oldCamPoint = cam->getCenter();
    Point oldLookat = cam->getLookat();
    cam->resetPosition( false );
    cam->doGLLookat();

    // note this still uses the old screen rect - assumes it stays on the same
    // plane, since the raytrace method ignores the rect boundaries
    Point topRight, bottomLeft;
    glUtil->screenToRectIntersect( (GLdouble)windowWidth,
                                   (GLdouble)windowHeight,
                                   getScreenRect( true ), topRight );
    glUtil->screenToRectIntersect( 0.0f, 0.0f, getScreenRect( true ),
                                    bottomLeft );

    screenL = bottomLeft.getX();
    screenR = topRight.getX();
    screenU = topRight.getY();
    screenD = bottomLeft.getY();

    cam->setCenter( oldCamPoint );
    cam->setLookat( oldLookat );
    cam->doGLLookat();

    screenRectFull.setPos( (screenL+screenR)/2.0f, (screenU+screenD)/2.0f);
    screenRectFull.setScale( screenR-screenL, screenU-screenD );

    recalculateRectSizes();

    if ( venueClientController != NULL )
    {
        venueClientController->setScale( screenRectSub.getDestWidth() * 0.9f,
                                     screenRectSub.getDestHeight() * 0.9f,
                                     true );
    }

    sessionManager->recalculateSize();
}

void ObjectManager::recalculateRectSizes()
{
    Bounds screenBounds = screenRectFull.getDestBounds();
    // if we have header, make the sub rectangle smaller accordingly
    // same for moving runway down, and shifting sub to the right because of the
    // runway
    float top = screenBounds.U;
    float left = screenBounds.L;
    if ( useHeader )
        top = screenBounds.U - ( 2.0f * textOffset ) -
                ( ( headerTextBox.Upper().Yf() - headerTextBox.Lower().Yf() )
                    * textScale );

    // resize the runway to go on the left side
    // TODO: make this work for horizontal, vertical, let user change it
    runway->setScale( ( screenBounds.R - screenBounds.L ) * 0.07f,
                      ( top - screenBounds.D ) * 0.93f, true );
    runway->setPos( screenBounds.L + ( ( top - screenBounds.D ) * 0.035f) +
                        runway->getDestWidth() / 2.0f,
                    ( top + screenBounds.D ) / 2.0f );

    if ( useRunway )
       left = screenBounds.L + ( ( top - screenBounds.D ) * 0.035f * 2.0f) +
               ( ( screenBounds.R - screenBounds.L ) * 0.07f );
    screenRectSub.setPos( ( left + screenBounds.R ) / 2.0f,
                          ( top + screenBounds.D ) / 2.0f );
    screenRectSub.setScale( screenBounds.R - left, top - screenBounds.D );
}

bool ObjectManager::usingSiteIDGroups()
{
    return enableSiteIDGroups;
}

void ObjectManager::setSiteIDGrouping( bool site )
{
    enableSiteIDGroups = site;
    if ( !site )
    {
        ungroupSiteIDGroups();
    }
}

int ObjectManager::getHoldCounter()
{
    return holdCounter;
}

void ObjectManager::resetAutoCounter()
{
    autoCounter = 0;
}

void ObjectManager::scaleSelectedObjects( float scaleAmt )
{
    for ( unsigned int i = 0; i < selectedObjects->size(); i++ )
    {
        RectangleBase* temp = (*selectedObjects)[i];
        temp->setScale( temp->getScaleX()+temp->getScaleX()*scaleAmt,
                         temp->getScaleY()+temp->getScaleY()*scaleAmt );
    }
}

std::vector<VideoSource*>* ObjectManager::getSources()
{
    return sources;
}

std::vector<RectangleBase*>* ObjectManager::getDrawnObjects()
{
    return drawnObjects;
}

std::vector<RectangleBase*>* ObjectManager::getSelectedObjects()
{
    return selectedObjects;
}

std::map<std::string,Group*>* ObjectManager::getSiteIDGroups()
{
    return siteIDGroups;
}

std::vector<RectangleBase*> ObjectManager::getMovableObjects()
{
    std::vector<RectangleBase*> objects;
    for ( unsigned int i = 0; i < drawnObjects->size(); i++ )
    {
        if ( !(*drawnObjects)[i]->isGrouped() &&
                (*drawnObjects)[i]->isUserMovable() )
        {
            objects.push_back( (*drawnObjects)[i] );
        }
    }
    return objects;
}

std::vector<RectangleBase*> ObjectManager::getUnselectedObjects()
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

void ObjectManager::addNewSource( VideoSource* s )
{
    if ( s == NULL ) return;

    Texture t = GLUtil::getInstance()->getTexture( "border" );
    s->setTexture( t.ID, t.width, t.height );

    lockSources();

    sources->push_back( s );
    drawnObjects->push_back( s );
    s->updateName();

    // tree add needs to be done on main thread since WX accesses the tree in
    // other places (ie, not thread safe, and this could be on a separate
    // thread)
    if ( tree != NULL )
        objectsToAddToTree->push_back( (RectangleBase*)s );

    // do extra placement stuff:
    // execute automatic mode layout again if it's on...
    if ( autoFocusRotate )
    {
        std::vector<RectangleBase*> tempOuterObjs = getMovableObjects();
        std::vector<RectangleBase*> tempInnerObj( tempOuterObjs.end()-1,
                                                    tempOuterObjs.end() );
        tempOuterObjs.erase( tempOuterObjs.end()-1 );

        std::map<std::string, std::vector<RectangleBase*> > data =
            std::map<std::string, std::vector<RectangleBase*> >();
        data["inners"] = tempInnerObj;
        data["outers"] = tempOuterObjs;
        layouts->arrange( "aspectFocus", getScreenRect(), RectangleBase(),
                            data );
    }
    // ...or rearrange it as a grid if the option is set...
    else if ( gridAuto )
    {
        std::map<std::string, std::vector<RectangleBase*> > data =
            std::map<std::string, std::vector<RectangleBase*> >();
        data["objects"] = getMovableObjects();
        layouts->arrange("grid", getScreenRect(), getEarthRect(), data);
    }
    // ...or if session focus is enabled, put videos from focused session in
    // center...
    else if ( sessionFocus )
    {
        if ( s->getSession()->getAddress().compare( focusSession ) == 0 )
        {
            sessionFocusObjs.push_back( s );
        }

        std::vector<RectangleBase*> tempOuterObjs = getMovableObjects();
        std::vector<RectangleBase*>::iterator sfo;
        for ( sfo = sessionFocusObjs.begin(); sfo != sessionFocusObjs.end();
                ++sfo )
        {
            std::vector<RectangleBase*>::iterator to;
            to = tempOuterObjs.begin();
            while ( to != tempOuterObjs.end() )
            {
                if ( (*sfo) == (*to) )
                {
                    to = tempOuterObjs.erase( to );
                }
                else
                {
                    ++to;
                }
            }
        }

        std::map<std::string, std::vector<RectangleBase*> > data =
            std::map<std::string, std::vector<RectangleBase*> >();
        data["inners"] = sessionFocusObjs;
        data["outers"] = tempOuterObjs;
        layouts->arrange( "aspectFocus", getScreenRect(), RectangleBase(),
                            data );
    }
    // otherwise add to runway if we're using it & have >9 videos
    else if ( useRunway && videoListener->getSourceCount() > 9 )
        runway->add( s );
    // base case will just use placement defined in VideoListener (9 grid with
    // stacking)

    unlockSources();
}

void ObjectManager::deleteSource( std::vector<VideoSource*>::iterator si )
{
    lockSources();

    RectangleBase* temp = (RectangleBase*)(*si);
    VideoSource* s = *si;

    removeFromLists( temp );

    sources->erase( si );

    // TODO need case for runway grouping?
    if ( temp->isGrouped() )
    {
        Group* g = temp->getGroup();

        // remove object from the group, regardless of whether it's a siteID
        // group or not.
        // this should work for runways, but should be more generic, ie for
        // groups of groups? maybe in removefromlists, but careful not to
        // degroup object before it hits that siteID check above or siteIDgroups
        // will have invalid references
        g->remove( temp );

        // delete the group the object was in if this is the last object in it
        // and it's an automatically made siteID group
        // TODO probably have a better metric for determining auto-siteID groups
        if ( g->getSiteID().compare( "" ) != 0 && g->numObjects() == 0 )
        {
            // note this duplicates the deleteGroup function since that does
            // mutex locking itself, and we already did that here
            removeFromLists( (RectangleBase*)g );

            // remove the group from the list of siteIDgroups
            std::map<std::string,Group*>::iterator gi =
                    siteIDGroups->find( g->getSiteID() );
            siteIDGroups->erase( gi );

            // put off the group delete, since removing it from the tree has to
            // happen on the main thread, and can't delete it before we remove
            // it from the tree
            objectsToDelete->push_back( g );
        }
    }

    if ( gridAuto )
    {
        std::map<std::string, std::vector<RectangleBase*> > data =
            std::map<std::string, std::vector<RectangleBase*> >();
        data["objects"] = getMovableObjects();
        layouts->arrange("grid", getScreenRect(), getEarthRect(), data);
    }

    // we need to do videosource's delete somewhere else, since this function
    // might be on a second thread, which would crash since the videosource
    // delete needs to do a GL call to delete its texture and GL calls can only
    // be on the main thread
    objectsToDelete->push_back( s );

    unlockSources();
}

void ObjectManager::deleteGroup( Group* g )
{
    lockSources();

    g->removeAll();
    removeFromLists( g );
    // need to delete groups later as well, since we need to remove them from
    // the tree first
    objectsToDelete->push_back( g );

    unlockSources();
}

void ObjectManager::addToDrawList( RectangleBase* obj )
{
    drawnObjects->push_back( obj );
}

void ObjectManager::removeFromLists( RectangleBase* obj, bool treeRemove )
{
    // remove it from the tree
    if ( tree && treeRemove )
    {
        objectsToRemoveFromTree->push_back( obj );
    }

    // remove it from drawnobjects, if it is being drawn
    std::vector<RectangleBase*>::iterator i = drawnObjects->begin();
    while ( i != drawnObjects->end() && (*i) != obj ) i++;

    // TODO: confirm this (checking whether it actually is in drawnobjects
    //                      or not)
    if ( i != drawnObjects->end() )
        drawnObjects->erase( i );

    // same for session focus objs
    i = sessionFocusObjs.begin();
    while ( i != sessionFocusObjs.end() && (*i) != obj ) i++;
    if ( i != sessionFocusObjs.end() )
        sessionFocusObjs.erase( i );

    if ( obj->isSelected() )
    {
        std::vector<RectangleBase*>::iterator j =
            selectedObjects->begin();
        while ( (*j) != obj ) j++;
        selectedObjects->erase( j );
    }
}

Group* ObjectManager::createSiteIDGroup( std::string data )
{
    gravUtil::logVerbose( "ObjectManager::creating siteIDGroup based on %s\n",
            data.c_str() );

    Group* g = new Group( 0.0f, 0.0f );
    g->setName( data );
    g->setSiteID( data );
    Texture t = GLUtil::getInstance()->getTexture( "border" );
    g->setTexture( t.ID, t.width, t.height );

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

/*
 * These use the destination center for the camera so that if you move the
 * camera while it is being moved, the new point is relative to the destination
 * point rather than the actual current point, which is in between the old point
 * and the current destination, and moving based on that can cause new points
 * to be off from what you might expect.
 */
float ObjectManager::getCamX()
{
    return cam->getDestCenter().getX();
}

float ObjectManager::getCamY()
{
    return cam->getDestCenter().getY();
}

float ObjectManager::getCamZ()
{
    return cam->getDestCenter().getZ();
}

void ObjectManager::setCamX( float x )
{
    Point p = cam->getDestCenter();
    p.setX( x );
    cam->moveCenter( p );
}

void ObjectManager::setCamY( float y )
{
    Point p = cam->getDestCenter();
    p.setY( y );
    cam->moveCenter( p );
}

void ObjectManager::setCamZ( float z )
{
    Point p = cam->getDestCenter();
    p.setZ( z );
    cam->moveCenter( p );
}

void ObjectManager::resetCamPosition()
{
    cam->resetPosition( true );
}

void ObjectManager::toggleOrbit()
{
    orbiting = !orbiting;
    orbiting ? orbitVideos() : resetOrbit();
}

void ObjectManager::orbitVideos()
{
    // zoom in cam a bit
    cam->resetPosition( true );
    Point c = cam->getDestCenter();
    c.setZ( c.getZ() - 5.0f );
    cam->moveCenter( c );

    std::vector<RectangleBase*> objs = getMovableObjects();
    std::vector<RectangleBase*>::iterator i;

    for ( i = objs.begin(); i != objs.end(); ++i )
    {
        (*i)->setScale( 5.0f, 5.0f );

        // move to lat/lon position
        Point p = earth->convertLatLong( (*i)->getLat(), (*i)->getLon(), true );
        (*i)->move( p );

        // get a vector from the rect to the earth projected onto the XZ plane
        // (ie, ignore Y) to compare with the regular rect->earth vector in
        // order to find the angles to rotate
        Vector proj( earth->getX() - (*i)->getDestX(), 0.0f,
                earth->getZ() - (*i)->getDestZ() );
        proj.normalize();

        // regular vector from rect to earth
        Vector earthDir = earth->getPos() - (*i)->getDestPos();
        earthDir.normalize();

        // forward lookat vector for rect, opposite of its normal since they
        // start out facing away from the earth
        Vector lookAt = (*i)->getNormal() * -1.0f;
        lookAt.normalize();

        // move obj out from earth a bit
        Vector moveVec = ( (*i)->getDestPos() - earth->getPos() ) * 0.2f;
        p = p + moveVec;
        (*i)->move( p );

        float yaw = acos( lookAt.dotProduct( proj ) ) * 180.0f / PI;
        float tilt = acos( earthDir.dotProduct( proj ) ) * 180.0f / PI;

        // bit of a hack, this could use some work
        if ( (*i)->getDestX() < earth->getX() )
        {
            yaw *= -1.0f;
        }
        if ( (*i)->getDestY() > earth->getY() )
        {
            tilt *= -1.0f;
        }
        if ( (*i)->getDestZ() < earth->getZ() )
        {
            tilt *= -1.0f;
        }

        // tilt not used for now, acting a bit weird
        (*i)->setRotation( 0.0f, yaw, 0.0f );
    }
}

void ObjectManager::resetOrbit()
{
    cam->resetPosition( true );

    std::vector<RectangleBase*> objs = getMovableObjects();
    std::vector<RectangleBase*>::iterator i;

    for ( i = objs.begin(); i != objs.end(); ++i )
    {
        (*i)->move( (*i)->getDestX(), (*i)->getDestY(), 0.0f );
        (*i)->setRotation( 0.0f, 0.0f, 0.0f );
    }
}

bool ObjectManager::isOrbiting()
{
    return orbiting;
}

void ObjectManager::rotateEarth( float x, float y, float z )
{
    earth->rotate( x, y, z );
    if ( orbiting )
    {
        orbitVideos();
    }
}

RectangleBase ObjectManager::getScreenRect( bool full )
{
    if ( full )
        return screenRectFull;
    else
        return screenRectSub;
}

RectangleBase ObjectManager::getEarthRect()
{
    return earthRect;
}

void ObjectManager::setEarth( Earth* e )
{
    earth = e;
}

void ObjectManager::setInput( InputHandler* i )
{
    input = i;
}

void ObjectManager::setTree( TreeControl* t )
{
    tree = t;
}

void ObjectManager::setAudio( AudioManager* a )
{
    if ( a != NULL )
        audioEnabled = true;
    else
        audioEnabled = false;

    audio = a;
}

void ObjectManager::setVideoListener( VideoListener* v )
{
    videoListener = v;
}

void ObjectManager::setVenueClientController( VenueClientController* vcc )
{
    // if setting a new one, stop drawing the old one
    if ( venueClientController != NULL )
    {
        std::vector<RectangleBase*>::iterator i = drawnObjects->begin();
        while ( i != drawnObjects->end() && (*i) != venueClientController )
            i++;
        drawnObjects->erase( i );
    }
    venueClientController = vcc;
    if ( venueClientController != NULL)
    {
        drawnObjects->push_back( venueClientController );
    }
}

void ObjectManager::setSessionManager( SessionManager* s )
{
    sessionManager = s;
    drawnObjects->push_back( sessionManager );
}

void ObjectManager::setHeaderString( std::string h )
{
    headerString = h;
    useHeader = headerString.compare( "" ) != 0;

    // since BBox may do some GL calls, any calls to this function must be on
    // the main thread
    if ( GLUtil::getInstance()->getMainFont() )
        headerTextBox = GLUtil::getInstance()->getMainFont()->BBox(
                            headerString.c_str() );

    recalculateRectSizes();
}

TreeControl* ObjectManager::getTree()
{
    return tree;
}

void ObjectManager::lockSources()
{
    if ( usingThreads )
    {
        mutex_lock( sourceMutex );
        lockCount++;
    }
}

void ObjectManager::unlockSources()
{
    if ( usingThreads )
    {
        mutex_unlock( sourceMutex );
        lockCount--;
    }
}

void ObjectManager::setThreads( bool threads )
{
    usingThreads = threads;
}

bool ObjectManager::usingRunway()
{
    return useRunway;
}

bool ObjectManager::usingGridAuto()
{
    return gridAuto;
}

bool ObjectManager::usingAutoFocusRotate()
{
    return autoFocusRotate;
}

void ObjectManager::setRunwayUsage( bool run )
{
    useRunway = run;

    runway->show( run );

    recalculateRectSizes();
}

void ObjectManager::setGridAuto( bool g )
{
    gridAuto = g;
    if ( g && useRunway )
        setRunwayUsage( false );
}

void ObjectManager::setAutoFocusRotate( bool a )
{
    autoFocusRotate = a;
    if ( a && useRunway )
        setRunwayUsage( false );
}

Runway* ObjectManager::getRunway()
{
    return runway;
}

void ObjectManager::setGraphicsDebugMode( bool g )
{
    graphicsDebugView = g;
    GLUtil::getInstance()->getCanvas()->setDebugTimerUsage( g );
}

bool ObjectManager::getGraphicsDebugMode()
{
    return graphicsDebugView;
}

void ObjectManager::toggleShowVenueClientController()
{
    if ( venueClientController != NULL )
    {
        venueClientController->show( !venueClientController->isShown() );
    }
}

bool ObjectManager::isVenueClientControllerShown()
{
    if ( venueClientController != NULL )
    {
        return venueClientController->isShown();
    }
    else
    {
        return false;
    }
}

bool ObjectManager::isVenueClientControllerShowable()
{
    if ( venueClientController != NULL )
    {
        return venueClientController->tryGetValidVenueClient();
    }
    else
    {
        return false;
    }
}

void ObjectManager::setThumbnailMap( std::map<std::string, std::string> tm )
{
    thumbnailMap = tm;
    sessionFocus = true;
}

void ObjectManager::setFocusSession( std::string f )
{
    focusSession = f;
    sessionFocusObjs.clear();
}

bool ObjectManager::audioAvailable()
{
    return audioEnabled && audio->getSourceCount() > 0;
}

void ObjectManager::doDelayedDelete()
{
    if ( objectsToDelete->size() > 0 )
    {
        for ( unsigned int i = 0; i < objectsToDelete->size(); i++ )
        {
            delete (*objectsToDelete)[i];
        }
        objectsToDelete->clear();
    }
}
