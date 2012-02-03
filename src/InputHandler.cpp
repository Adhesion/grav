/*
 * @file InputHandler.h
 *
 * Implementation of a class for processing keyboard & mouse input and managing
 * selection & movement of objects accordingly.
 *
 * @author Andrew Ford
 * @author Ralph Bean
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

#include "gravUtil.h"
#include "InputHandler.h"
#include "GLUtil.h"
#include "VideoSource.h"
#include "RectangleBase.h"
#include "Group.h"
#include "ObjectManager.h"
#include "Earth.h"
#include "Frame.h"
#include "Runway.h"

#include <VPMedia/random_helper.h>

/* For key formatting */
#include <sstream>
#include <iomanip>

int InputHandler::propertyID = wxNewId();

BEGIN_EVENT_TABLE(InputHandler, wxEvtHandler)
EVT_KEY_DOWN(InputHandler::wxKeyDown)
EVT_CHAR(InputHandler::wxCharEvt)
EVT_MOTION(InputHandler::wxMouseMove)
EVT_LEFT_DOWN(InputHandler::wxMouseLDown)
EVT_LEFT_DCLICK(InputHandler::wxMouseLDClick)
EVT_LEFT_UP(InputHandler::wxMouseLUp)
EVT_RIGHT_DOWN(InputHandler::wxMouseRDown)
END_EVENT_TABLE();

InputHandler::InputHandler( Earth* e, ObjectManager* o, Frame* f )
    : earth( e ), objectMan( o ), mainFrame( f )
{
    tempSelectedObjects = new std::vector<RectangleBase*>();
    dragging = false;
    clickedInside = false;
    leftButtonHeld = false;
    validMousePos = false;
    ctrlHeld = false;
    modifiers = 0;
    bool debug = false;
#ifdef GRAV_DEBUG_MODE
    debug = true;
#endif

    // Here we register which keys do what (declarative programming).
    /* Misc Management */
    lookup[ktoh('H')] = &InputHandler::handleHelp;
    docstr[ktoh('H')] = "Print this help message to the commandline.";
    lookup[ktoh('T')] = &InputHandler::handleRearrangeGroups;
    docstr[ktoh('T')] = "Rearrange groups.";
    lookup[ktoh('U')] = &InputHandler::handleUpdateGroupNames;
    docstr[ktoh('U')] = "Update group names.";
    lookup[ktoh('L')] = &InputHandler::handleToggleGroupLocks;
    docstr[ktoh('L')] = "Toggle group locks.";
    lookup[ktoh('G')] = &InputHandler::handleToggleSiteGrouping;
    docstr[ktoh('G')] = "Toggle site grouping.";
    lookup[ktoh('X')] = &InputHandler::handleToggleRenderingSelected;
    docstr[ktoh('X')] = "Toggle rendering of selected objects.";
    lookup[ktoh('Q', wxMOD_CMD)] = &InputHandler::handleQuit;
    docstr[ktoh('Q', wxMOD_CMD)] = "Quit.";
    lookup[ktoh('\e')] = &InputHandler::handleQuit;
    docstr[ktoh('\e')] = "Quit.";
    unprintables['\e'] = "(escape)";
    lookup[ktoh((unsigned char)13, wxMOD_ALT)] =
                        &InputHandler::handleToggleFullscreen;
    docstr[ktoh((unsigned char)13, wxMOD_ALT)] = "Toggle fullscreen.";
    unprintables[(unsigned char)13] = "(enter)";
    lookup[ktoh('R', wxMOD_ALT)] =
                        &InputHandler::handleRunwayToggle;
    docstr[ktoh('R', wxMOD_ALT)] = "Toggle runway visibility.";
    lookup[ktoh('V', wxMOD_CMD)] =
                        &InputHandler::handleToggleShowVenueClientController;
    docstr[ktoh('V', wxMOD_CMD)] = "Toggle venue client controller visibility.";

    /* Selection */
    lookup[ktoh('A', wxMOD_CMD)] = &InputHandler::handleSelectAll;
    docstr[ktoh('A', wxMOD_CMD)] = "Select all.";
    lookup[ktoh('I', wxMOD_CMD)] = &InputHandler::handleInvertSelection;
    docstr[ktoh('I', wxMOD_CMD)] = "Invert selection.";
    lookup[ktoh('\b')] = &InputHandler::handleClearSelected;
    docstr[ktoh('\b')] = "Clear selection.";
    unprintables['\b'] = "(backspace)";

    /* Misc Manipulation */
    lookup[ktoh('-')] = &InputHandler::handleDownscaleSelected;
    docstr[ktoh('-')] = "Downscale selected objects.";
    lookup[ktoh('+')] = &InputHandler::handleUpscaleSelected; // TDB -- dup?
    docstr[ktoh('+')] = "Upscale selected objects.";
    lookup[ktoh('=')] = &InputHandler::handleUpscaleSelected; // TBD -- dup?
    docstr[ktoh('=')] = "Upscale selected objects.";
    lookup[ktoh('F', wxMOD_SHIFT)] =
                        &InputHandler::handleFullscreenSelectedSingle;
    docstr[ktoh('F', wxMOD_SHIFT)] =
                "Fullscreen selected object (includes border and text).";
    lookup[ktoh('F', wxMOD_SHIFT | wxMOD_CMD)] =
                        &InputHandler::handleFullerFullscreenSelectedSingle;
    docstr[ktoh('F', wxMOD_SHIFT | wxMOD_CMD)] =
                "Fullscreen selected object (video/inner contents of object).";
    lookup[ktoh('M')] = &InputHandler::handleMuteSelected;
    docstr[ktoh('M')] = "Mute selected objects.";
    lookup[ktoh('N')] = &InputHandler::handleNativeScaleSelected;
    docstr[ktoh('N')] = "Scale selected videos to native size.";
    lookup[ktoh('N', wxMOD_SHIFT)] = &InputHandler::handleNativeScaleAll;
    docstr[ktoh('N', wxMOD_SHIFT)] = "Scale all videos to native size.";

    /* Different Layouts */
    lookup[ktoh('P')] = &InputHandler::handlePerimeterArrange;
    docstr[ktoh('P')] = "Arrange objects around the perimeter of the screen.";
    lookup[ktoh('R')] = &InputHandler::handleGridArrange;
    docstr[ktoh('R')] = "Arrange objects into a grid.";
    lookup[ktoh('F')] = &InputHandler::handleFocusArrange;
    docstr[ktoh('F')] = "Rearrange objects to focus on selected objects.";
    lookup[ktoh('A', wxMOD_ALT)] = &InputHandler::handleToggleAutoFocusRotate;
    docstr[ktoh('A', wxMOD_ALT)] = "Toggle 'automatic' mode (rotating focus)";

    lookup[ktoh('D', wxMOD_SHIFT | wxMOD_CMD)] =
                        &InputHandler::handleToggleGraphicsDebug;
    docstr[ktoh('D', wxMOD_SHIFT | wxMOD_CMD)] =
                        "Toggle graphics debugging information.";

    if ( debug ) {
        /* Debug keys */
        lookup[ktoh(' ')] = &InputHandler::handleAddTestObject;
        docstr[ktoh(' ')] = "[debug] Add a test window to the screen.";
        unprintables[' '] = "(space)";
        lookup[ktoh((unsigned char)127)] = &InputHandler::handleTryDeleteObject;
        docstr[ktoh((unsigned char)127)] = "[debug] Delete selected deletable"
                                            " objects (test objects)";
        unprintables[(unsigned char)127] = "(delete)";
        lookup[ktoh('K')] = &InputHandler::handlePrintSelected;
        docstr[ktoh('K')] = "[debug] Print information about selected windows.";
        lookup[ktoh('O')] = &InputHandler::handleRandomTest;
        docstr[ktoh('O')] = "[debug] Print out randomly generated numbers.";
        lookup[ktoh('0')] = &InputHandler::handleMoveAllToCenter;
        docstr[ktoh('0')] = "[debug] Move all objects to the center.";
        lookup[ktoh('I')] = &InputHandler::handleInformation;
        docstr[ktoh('I')] = "[debug] Print information about all objects.";

        /* Navigation */
        lookup[ktoh('A')] = &InputHandler::handleStrafeLeft;
        docstr[ktoh('A')] = "[debug] Strafe left.";
        lookup[ktoh('D')] = &InputHandler::handleStrafeRight;
        docstr[ktoh('D')] = "[debug] Strafe right.";
        lookup[ktoh('W')] = &InputHandler::handleZoomin;
        docstr[ktoh('W')] = "[debug] Zoom in.";
        lookup[ktoh('S')] = &InputHandler::handleZoomout;
        docstr[ktoh('S')] = "[debug] Zoom out.";
        lookup[ktoh('C', wxMOD_SHIFT | wxMOD_CMD)] =
                            &InputHandler::handleCameraReset;
        docstr[ktoh('C', wxMOD_SHIFT | wxMOD_CMD)] =
                            "[debug] Reset camera position.";
    }
}

InputHandler::~InputHandler()
{
    delete tempSelectedObjects;
    // all other pointers are owned by the main class
}

void InputHandler::wxKeyDown( wxKeyEvent& evt )
{
    /*shiftHeld = ( evt.GetModifiers() == wxMOD_SHIFT );
    altHeld = ( evt.GetModifiers() == wxMOD_ALT );
    ctrlHeld = ( evt.GetModifiers() == wxMOD_CMD );*/
    modifiers = evt.GetModifiers();

    processKeyboard( evt.GetKeyCode(), 0, 0 );

    evt.Skip(); // so now the char event can grab this, if need be
}

void InputHandler::wxCharEvt( wxKeyEvent& evt )
{
    // disabled temporarily - using keydown for everything
}

void InputHandler::wxMouseMove( wxMouseEvent& evt )
{
    // update mouse world position on mouse move - adds negligible CPU (~1%) and
    // obviously only when the mouse is moving. if need be, can potentially be
    // put off to ObjectManager::draw() (ie every X frames)

    // note that calculating screen pos -> world pos needs to do a GL call so
    // this has to be on the main thread

    int x = evt.GetPosition().x;
    // GL screen coords are y-flipped relative to GL screen coords
    int y = objectMan->getWindowHeight() - evt.GetPosition().y;

    Point intersect;
    validMousePos = GLUtil::getInstance()->screenToRectIntersect( x, y,
            objectMan->getScreenRect(), intersect );
    if ( !validMousePos )
        return;

    mouseX = intersect.getX();
    mouseY = intersect.getY();

    if ( leftButtonHeld )
        mouseLeftHeldMove();
}

void InputHandler::wxMouseLDown( wxMouseEvent& evt )
{
    // TODO fix these? how to best handle mouse modifiers?
    if ( evt.CmdDown() )
        ctrlHeld = true;
    else
        ctrlHeld = false;
    //modifiers = evt.GetModifiers();

    leftClick();
    evt.Skip();
}

void InputHandler::wxMouseLUp( wxMouseEvent& evt )
{
    leftRelease();
    evt.Skip();
}

void InputHandler::wxMouseLDClick( wxMouseEvent& evt )
{
    // TODO same as above
    if ( evt.CmdDown() )
        ctrlHeld = true;
    else
        ctrlHeld = false;
    //modifiers = evt.GetModifiers();

    leftClick( true );
    evt.Skip();
}

void InputHandler::wxMouseRDown( wxMouseEvent& evt )
{
    if ( objectMan->getSelectedObjects()->size() > 0 )
    {
        wxMenu rightClickMenu;
        rightClickMenu.Append( propertyID, _("Properties") );
        mainFrame->PopupMenu( &rightClickMenu, evt.GetPosition() );
    }
}

void InputHandler::handlePrintSelected()
{
    gravUtil::logMessage( "InputHandler::current sources selected: %i\n",
            objectMan->getSelectedObjects()->size() );
    for ( unsigned int i = 0; i < objectMan->getSelectedObjects()->size(); i++ )
    {
        gravUtil::logMessage( "%s\n",
                (*(objectMan->getSelectedObjects()))[i]->getName().c_str() );
    }
}

void InputHandler::handleRearrangeGroups()
{
    gravUtil::logMessage( "InputHandler::rearranging groups...\n" );
    for ( unsigned int i = 0; i < objectMan->getSelectedObjects()->size(); i++ )
    {
        RectangleBase* temp = (*(objectMan->getSelectedObjects()))[i];
        if ( temp->isGroup() )
        {
            Group* g = (Group*)temp;
            g->rearrange();
        }
    }
}

void InputHandler::handleUpdateGroupNames()
{
    gravUtil::logMessage( "InputHandler::updating group names...\n" );
    std::map<std::string,Group*>::iterator mapi;
    mapi = objectMan->getSiteIDGroups()->begin();
    for ( ; mapi != objectMan->getSiteIDGroups()->end(); mapi++ )
    {
        mapi->second->updateName();
    }
}

void InputHandler::handlePerimeterArrange()
{
    std::map<std::string, std::vector<RectangleBase*> > data;
    data["objects"] = objectMan->getMovableObjects();
    layouts.arrange( "perimeter", objectMan->getScreenRect(),
            objectMan->getEarthRect(), data );
}

void InputHandler::handleGridArrange()
{
    std::map<std::string, std::vector<RectangleBase*> > data;
    data["objects"] = objectMan->getMovableObjects();
    layouts.arrange( "grid", objectMan->getScreenRect(),
            RectangleBase(), data );
}

void InputHandler::handleFocusArrange()
{
    if ( objectMan->getSelectedObjects()->size() > 0 )
    {
        std::map<std::string, std::vector<RectangleBase*> > data;
        data["outers"] = objectMan->getUnselectedObjects();
        data["inners"] = *(objectMan->getSelectedObjects());
        layouts.arrange( "aspectFocus", objectMan->getScreenRect(),
                RectangleBase(), data );
    }
}

void InputHandler::handleFullscreenSelectedSingle()
{
    if ( objectMan->getSelectedObjects()->size() == 1 )
    {
        (*objectMan->getSelectedObjects())[0]->fillToRect(
                objectMan->getScreenRect() );
    }
}

void InputHandler::handleFullerFullscreenSelectedSingle()
{
    if ( objectMan->getSelectedObjects()->size() == 1 )
    {
        (*objectMan->getSelectedObjects())[0]->fillToRect(
                objectMan->getScreenRect( true ), true );
    }
}

void InputHandler::handleRunwayToggle()
{
    objectMan->setRunwayUsage( !objectMan->usingRunway() );
    objectMan->clearSelected();
}

void InputHandler::handleInvertSelection()
{
    std::vector<RectangleBase*> movableObjects = objectMan->getMovableObjects();
    for ( unsigned int i = 0; i < movableObjects.size(); i++ )
    {
        RectangleBase* obj = movableObjects[i];
        obj->setSelect( !obj->isSelected() );
        if ( obj->isSelected() )
        {
            objectMan->getSelectedObjects()->push_back( obj );
        }
        else
        {
            std::vector<RectangleBase*>::iterator it =
                    objectMan->getSelectedObjects()->begin();
            while ( (*it) != obj ) it++;
            objectMan->getSelectedObjects()->erase( it );
        }
    }
}

void InputHandler::handleHelp()
{
    std::map<std::string, std::string>::const_iterator i;
    std::map<std::string, std::string> helpList = getShortcutHelpList();
    gravUtil::logMessage( "InputHandler::List of mapped keys:\n" );
    for ( i = helpList.begin(); i != helpList.end(); ++i )
    {
        std::ostringstream sstr;
        sstr << std::setw(25) << i->first;
        gravUtil::logMessage( "%s\t%s\n", sstr.str().c_str(),
                i->second.c_str() );
    }
}

void InputHandler::handleInformation()
{
    std::vector<VideoSource*>::const_iterator si;
    gravUtil::logMessage( "InputHandler::printing source/object info...\n" );
    gravUtil::logMessage( "\tScreen size is %f x %f\n",
            objectMan->getScreenRect().getWidth(),
            objectMan->getScreenRect().getHeight() );
    gravUtil::logMessage( "\tWe currently have %i video sources.\n",
            objectMan->getSources()->size() );

    for ( si = objectMan->getSources()->begin();
            si != objectMan->getSources()->end(); si++ )
    {
        gravUtil::logMessage( "\t\tname: %s\n",
                (*si)->getMetadata( VPMSession::VPMSESSION_SDES_NAME).c_str() );
        gravUtil::logMessage( "\t\tstored name: %s\n",
                (*si)->getName().c_str() );
        gravUtil::logMessage( "\t\tcname: %s\n",
                (*si)->getMetadata(
                        VPMSession::VPMSESSION_SDES_CNAME).c_str() );
        gravUtil::logMessage( "\t\tstored altname: %s\n",
                (*si)->getAltName().c_str() );
        gravUtil::logMessage( "\t\tloc: %s\n",
                (*si)->getMetadata( VPMSession::VPMSESSION_SDES_LOC).c_str() );
        gravUtil::logMessage( "\t\tssrc 0x%08x (%d)\n", (*si)->getssrc(),
                (*si)->getssrc() );
        gravUtil::logMessage( "\t\tpos (world): %f,%f\n",
                (*si)->getX(), (*si)->getY() );
        GLdouble scrX; GLdouble scrY; GLdouble scrZ;
        GLUtil::getInstance()->worldToScreen( (GLdouble)(*si)->getX(),
                (GLdouble)(*si)->getY(),
                (GLdouble)(*si)->getZ(),
                &scrX, &scrY, &scrZ);
        gravUtil::logMessage( "\t\tpos (screen): %f,%f,%f\n",
                scrX, scrY, scrZ );
        gravUtil::logMessage( "\t\tis grouped? %i\n", (*si)->isGrouped() );
        gravUtil::logMessage( "\t\tDescription of codec: %s\n",
                (*si)->getPayloadDesc() );
        gravUtil::logMessage( "\t\tis muted? %i\n", (*si)->isMuted() );
        gravUtil::logMessage( "\t\tSize: %f x %f\n", (*si)->getWidth(),
                (*si)->getHeight() );
        gravUtil::logMessage( "\t\tText size: %f x %f\n", (*si)->getTextWidth(),
                (*si)->getTextHeight() );
        gravUtil::logMessage( "" );
    }

    gravUtil::logMessage( "\tWe currently have %i objects in drawnObjects.\n",
            objectMan->getDrawnObjects()->size() );
    for ( unsigned int i = 0; i < objectMan->getDrawnObjects()->size(); i++ )
    {
        RectangleBase* temp = (*(objectMan->getDrawnObjects()))[i];
        gravUtil::logMessage( "\t\t%s (%fx%f)\n", temp->getName().c_str(),
                temp->getDestWidth(), temp->getDestHeight() );
    }

    gravUtil::logMessage( "InputHandler::done printing source/object info.\n" );
}

void InputHandler::handleToggleGroupLocks()
{
    for ( unsigned int i = 0; i < objectMan->getSelectedObjects()->size(); i++ )
    {
        RectangleBase* temp = (*(objectMan->getSelectedObjects()))[i];
        if ( temp->isGroup() )
        {
            Group* g = (Group*)temp;
            gravUtil::logMessage( "InputHandler::group %s locked? %i\n",
                        g->getName().c_str(), g->isLocked() );
            g->changeLock();
            gravUtil::logMessage( "\tafter: %i\n", g->isLocked() );
        }
    }
}

void InputHandler::handleMuteSelected()
{
    std::vector<VideoSource*>::const_iterator si;
    for ( si = objectMan->getSources()->begin();
                    si != objectMan->getSources()->end(); ++si )
    {
        if ( (*si)->isSelected() )
        {
            (*si)->toggleMute();
            // add it to the runway if we're using it, it's visible, etc.
            if ( !(*si)->isGrouped() && (*si)->isMuted() &&
                    objectMan->usingRunway() &&
                    objectMan->getRunway()->isShown() )
            {
                objectMan->getRunway()->add( (*si) );
                (*si)->setSelect( false );
            }
        }
    }
    objectMan->clearSelected();
}

void InputHandler::handleRandomTest()
{
    gravUtil::logMessage( "InputHandler::random32: %i\n", random32() );
    gravUtil::logMessage( "InputHandler::random32max: %i\n", random32_max() );
}

void InputHandler::handleNativeScaleAll()
{
    std::vector<VideoSource*>::const_iterator si;
    for ( si = objectMan->getSources()->begin();
            si != objectMan->getSources()->end(); ++si )
    {
        (*si)->scaleNative();
        if ( (*si)->isGrouped() )
            (*si)->getGroup()->rearrange();
    }
}

void InputHandler::handleNativeScaleSelected()
{
    std::vector<VideoSource*>::const_iterator si;
    for ( si = objectMan->getSources()->begin();
            si != objectMan->getSources()->end(); ++si )
    {
        if ( (*si)->isSelected() )
        {
            (*si)->scaleNative();
            if ( (*si)->isGrouped() )
                (*si)->getGroup()->rearrange();
        }
    }
}

void InputHandler::handleMoveAllToCenter()
{
    std::vector<VideoSource*>::const_iterator si;
    for ( si = objectMan->getSources()->begin();
            si != objectMan->getSources()->end(); ++si )
    {
        (*si)->move( 0.0f, 0.0f );
    }
}

void InputHandler::handleToggleSiteGrouping()
{
    // TODO -- condense this with a bang
    if ( objectMan->usingSiteIDGroups() )
    {
        objectMan->setSiteIDGrouping( false );
        objectMan->ungroupAll();
    }
    else
    {
        objectMan->setSiteIDGrouping( true );
    }
}

void InputHandler::handleToggleShowVenueClientController()
{
    objectMan->toggleShowVenueClientController();
}

void InputHandler::handleToggleRenderingSelected()
{
    for ( unsigned int i = 0; i < objectMan->getSelectedObjects()->size();
                    i++ )
    {
        RectangleBase* temp = (*(objectMan->getSelectedObjects()))[i];
        VideoSource* tempVS = dynamic_cast<VideoSource*>( temp );
        if ( tempVS != NULL )
        {
            tempVS->setRendering( !tempVS->getRendering() );
        }
    }
}

void InputHandler::handleZoomout()
{
    objectMan->setCamZ(objectMan->getCamZ()+1);
}

void InputHandler::handleZoomin()
{
    objectMan->setCamZ(objectMan->getCamZ()-1);
}

void InputHandler::handleStrafeLeft()
{
    objectMan->setCamX(objectMan->getCamX()-1);
}

void InputHandler::handleStrafeRight()
{
    objectMan->setCamX(objectMan->getCamX()+1);
}

void InputHandler::handleCameraReset()
{
    objectMan->resetCamPosition();
}

void InputHandler::handleToggleAutoFocusRotate()
{
    objectMan->setAutoFocusRotate( !objectMan->usingAutoFocusRotate() );
    objectMan->resetAutoCounter();
}

void InputHandler::handleSelectAll()
{
    std::vector<RectangleBase*> movableObjects = objectMan->getMovableObjects();
    objectMan->clearSelected();
    for ( unsigned int i = 0; i < movableObjects.size(); i++ )
    {
        movableObjects[i]->setSelect( true );
        objectMan->getSelectedObjects()->push_back( movableObjects[i] );
    }
}

void InputHandler::handleToggleGraphicsDebug()
{
    objectMan->setGraphicsDebugMode( !objectMan->getGraphicsDebugMode() );
}

void InputHandler::handleDownscaleSelected()
{
    float scaleAmt = 0.25f;
    objectMan->scaleSelectedObjects( scaleAmt * -1.0f );
}

void InputHandler::handleUpscaleSelected()
{
    float scaleAmt = 0.25f;
    objectMan->scaleSelectedObjects( scaleAmt );
}

void InputHandler::handleToggleFullscreen()
{
    mainFrame->ShowFullScreen( !mainFrame->IsFullScreen() );
}

void InputHandler::handleQuit()
{
    mainFrame->Close();
}

void InputHandler::handleClearSelected()
{
    objectMan->clearSelected();
}

void InputHandler::handleAddTestObject()
{
    objectMan->addTestObject();
}

void InputHandler::handleTryDeleteObject()
{
    std::vector<RectangleBase*> movableObjects = objectMan->getMovableObjects();
    for ( unsigned int i = 0; i < movableObjects.size(); i++ )
    {
        RectangleBase* obj = movableObjects[i];
        if ( obj->isSelected() )
            objectMan->tryDeleteObject( obj );
    }
}

/* Key To Hash */
int InputHandler::ktoh( unsigned char key )
{
    return ktoh( key, 0 );
}

int InputHandler::ktoh( unsigned char key, int _modifiers )
{
    return ( ((int)key)  << 4 ) | _modifiers;
}

/* Hash To Key */
unsigned char InputHandler::htok( int hash )
{
    return (unsigned char)(hash >> 4);
}

/* Hash to String representation */
std::string InputHandler::htos( int hash )
{
    char key = htok( hash );
    std::map<char, std::string>::iterator upIter = unprintables.find( key );
    std::ostringstream sstr;

    std::string shi = (hash & wxMOD_SHIFT) ? "shift + " : "";
    std::string alt = (hash & wxMOD_ALT) ? "alt + " : "";
    std::string cmd = (hash & wxMOD_CMD) ? "ctrl + " : "";

    sstr << shi << alt << cmd;

    if ( upIter != unprintables.end() )
        sstr << unprintables[ key ];
    else
        sstr << key;

    return sstr.str();
}

void InputHandler::processKeyboard( int keyCode, int x, int y )
{
    std::vector<VPMVideoBufferSink*>::iterator t;
    unsigned char key = (unsigned char)keyCode;
    int hash = ktoh(key, this->modifiers);
    /*gravUtil::logMessage( "Char pressed is %c (%i)\n", key, key );
    gravUtil::logMessage( "keycode is %i\n", keyCode );
    gravUtil::logMessage( "x,y in processKeyboard is %i,%i\n", x, y );
    gravUtil::logMessage( "is shift (only) held? %i\n", modifiers == wxMOD_SHIFT );
    gravUtil::logMessage( "is ctrl (only) held? %i\n", modifiers == wxMOD_CMD );
    gravUtil::logMessage( "is alt (only) held? %i\n", modifiers == wxMOD_ALT );*/
    // how much to scale when doing -/+: flipped in the former case

    // Lookup the pressed key in our map of method pointers
    std::map<int, MFP>::iterator lookupIter;
    lookupIter = lookup.find(hash);
    if( lookupIter != lookup.end() )
        (this->*(lookup[hash]))();
    else
        gravUtil::logVerbose( "InputHandler::No handler for registered for key "
                "(code is %i):\n\t%s\n", keyCode, htos(hash).c_str());

    // TBD -- how do we reconcile this with the map?
    switch ( keyCode )
    {
    // u/d/l/r arrow keys, for WX
    // TODO: are these axes backwards?
    case WXK_UP:
        earth->rotate( -2.0f, 0.0f, 0.0f );
        break;
    case WXK_DOWN:
        earth->rotate( 2.0f, 0.0f, 0.0f );
        break;
    case WXK_LEFT:
        earth->rotate( 0.0f, 0.0f, -2.0f );
        break;
    case WXK_RIGHT:
        earth->rotate( 0.0f, 0.0f, 2.0f );
        break;
    }
}

void InputHandler::leftClick( bool doubleClick )
{
    if ( !validMousePos )
        return;

    // on click, any potential dragging afterwards must start here
    dragStartX = mouseX;
    dragStartY = mouseY;
    dragPrevX = mouseX;
    dragPrevY = mouseY;

    objectMan->setBoxSelectDrawing( false );

    // if we didn't click on a video, and we're not holding down ctrl to
    // begin a multi selection, so move the selected video(s) to the
    // clicked position in empty space
    selectVideos();
    if ( !clickedInside && !ctrlHeld )
    {
        if ( !objectMan->getSelectedObjects()->empty() )
        {
            float movePosX = mouseX; float movePosY = mouseY;
            std::vector<RectangleBase*>::const_iterator sli;

            float avgX = 0.0f, avgY = 0.0f;
            int num = objectMan->getSelectedObjects()->size();

            if ( num == 1 )
            {
                if ( (*(objectMan->getSelectedObjects()))[0]->isUserMovable() )
                {
                    (*(objectMan->getSelectedObjects()))[0]->move( movePosX,
                                                                movePosY );
                }
                (*(objectMan->getSelectedObjects()))[0]->setSelect( false );
            }
            // if moving >1, center the videos around the click point
            // we need to find the average pos beforehand
            else
            {
                // average the vals
                for ( sli = objectMan->getSelectedObjects()->begin();
                       sli != objectMan->getSelectedObjects()->end();
                       sli++ )
                {
                    avgX += (*sli)->getX();
                    avgY += (*sli)->getY();
                }
                avgX = avgX / num;
                avgY = avgY / num;

                // move and set them to be unselected
                for ( sli = objectMan->getSelectedObjects()->begin();
                       sli != objectMan->getSelectedObjects()->end();
                       sli++ )
                {
                    if ( (*sli)->isUserMovable() )
                    {
                        (*sli)->move( movePosX + ((*sli)->getX()-avgX),
                                       movePosY + ((*sli)->getY()-avgY) );
                    }
                    (*sli)->setSelect( false );
                }
            }
            objectMan->getSelectedObjects()->clear();
        }
    }
    // or if we did click on a video...
    else if ( doubleClick )
    {
        if ( objectMan->getSelectedObjects()->size() == 1 )
        {
            (*objectMan->getSelectedObjects())[0]->doubleClickAction();
        }
    }
    else // unused old stuff
    {
        // suppress the box selection
        //leftButtonHeld = false;
        //clickedInside = true;
    }

    // either way the left button is now held
    leftButtonHeld = true;
}

void InputHandler::leftRelease()
{
    // shift the temporary list of selected objects to the main list to
    // allow for multiple box selection
    for ( unsigned int i = 0; i < tempSelectedObjects->size(); i++ )
    {
        objectMan->getSelectedObjects()->push_back( (*tempSelectedObjects)[i] );
    }
    tempSelectedObjects->clear();
    leftButtonHeld = false;

    // if we were doing drag movement, deselect all
    // the getholdcounter check is to prevent user from losing a click selection
    // if they accidentally drag it a tiny bit
    if ( dragging && objectMan->getHoldCounter() > 5 )
    {
        objectMan->clearSelected();
    }

    // since there is no other way to drag (at the moment) without holding down
    // the mouse, we're necessarily not dragging anymore
    // (dragging in this case refers to dragging an object, not a select box)
    // putting this here rather than in the above if statement prevents box
    // selection from failing after an accidental small drag
    dragging = false;
}

void InputHandler::mouseLeftHeldMove()
{
    dragEndX = mouseX;
    dragEndY = mouseY;

    // set new position, when doing click-and-drag movement
    if ( clickedInside )
    {
        std::vector<RectangleBase*>::reverse_iterator sli;
        for ( sli = objectMan->getSelectedObjects()->rbegin();
                sli != objectMan->getSelectedObjects()->rend();
                sli++ )
        {
            // since group members are controlled by the group somewhat,
            // calling setpos on both groups and members here will result
            // in a double move (if they're both selected)
            if ( (*sli)->isUserMovable() &&
                 ( !(*sli)->isGrouped() || !(*sli)->getGroup()->isSelected() ) )
            {
                (*sli)->setPos( (dragEndX-dragPrevX) + (*sli)->getX(),
                                (dragEndY-dragPrevY) + (*sli)->getY() );
            }
        }
        objectMan->setBoxSelectDrawing( false );
        dragging = true;
    }
    // if we didn't click inside & we're holding the left button, do the box
    // selection (clearing the temp selected list to allow for the box
    // intersecting with an object at first, then not intersecting later
    else if ( leftButtonHeld )
    {
        for ( std::vector<RectangleBase*>::iterator sli =
                                    tempSelectedObjects->begin();
              sli != tempSelectedObjects->end(); sli++ )
            (*sli)->setSelect(false);
        tempSelectedObjects->clear();
        selectVideos();
        objectMan->setBoxSelectDrawing( true );
    }

    dragPrevX = mouseX;
    dragPrevY = mouseY;
}

bool InputHandler::selectVideos()
{
    bool videoSelected = false;

    std::vector<RectangleBase*>::reverse_iterator si;
    // reverse means we'll get the video that's on top first, since videos
    // later in the list will render on top of previous ones
    std::vector<RectangleBase*>::const_iterator sli;

    for ( si = objectMan->getDrawnObjects()->rbegin();
            si != objectMan->getDrawnObjects()->rend(); ++si )
    {
        // rectangle that defines the selection area
        float selectL, selectR, selectU, selectD;
        if ( leftButtonHeld )
        {
            selectL = std::min( dragStartX, dragEndX );
            selectR = std::max( dragStartX, dragEndX );
            selectD = std::min( dragStartY, dragEndY );
            selectU = std::max( dragStartY, dragEndY );
            //lastBoxed = true;
        }
        else
        {
            selectL = selectR = mouseX;
            selectU = selectD = mouseY;
            //lastBoxed = false;
        }

        bool intersect = (*si)->intersect( selectL, selectR, selectU, selectD )
                            && (*si)->isSelectable();

        // for click-and-drag movement
        clickedInside = intersect && !leftButtonHeld;

        if ( intersect )
        {
            // clear the list of selected if we're clicking on a new video
            if ( !leftButtonHeld && !(*si)->isSelected() && !ctrlHeld )
                objectMan->clearSelected();

            videoSelected = true;
            RectangleBase* temp = (*si);

            if ( !temp->isSelected() )
            {
                if ( temp->isGrouped() )
                {
                    // TODO change this to a loop to work for nested groups
                    Group* g = temp->getGroup();
                    if ( g->isLocked() )
                        temp = (RectangleBase*)g;
                }

                temp->setSelect( true );
                // if we're doing a box selection, add it to the temp list
                if ( leftButtonHeld )
                    tempSelectedObjects->push_back( temp );
                else
                    objectMan->getSelectedObjects()->push_back( temp );
            }
            // if what we just clicked on is selected and we're holding ctrl,
            // deselect
            else if ( ctrlHeld && !leftButtonHeld )
            {
                temp->setSelect( false );
                std::vector<RectangleBase*>::iterator sli =
                                    objectMan->getSelectedObjects()->begin();
                while ( (*sli) != temp )
                    sli++;
                objectMan->getSelectedObjects()->erase( sli );
            }

            // take the selected video and put it at the end of the list so
            // it'll be rendered on top - but only if we just clicked on it
            if ( !leftButtonHeld )
            {
                objectMan->lockSources();
                objectMan->moveToTop( temp );
                objectMan->unlockSources();

                break; // so we only select one video per click
                       // when single-clicking
            }
        }
        else
        {
            //gravUtil::logMessage( "InputHandler::selectVideos: did not intersect\n" );
        }
    }

    return videoSelected;
}

bool InputHandler::isLeftButtonHeld()
{
    return leftButtonHeld;
}

float InputHandler::getMouseX()
{
    return mouseX;
}

float InputHandler::getMouseY()
{
    return mouseY;
}

float InputHandler::getDragStartX()
{
    return dragStartX;
}

float InputHandler::getDragStartY()
{
    return dragStartY;
}

float InputHandler::getDragEndX()
{
    return dragEndX;
}

float InputHandler::getDragEndY()
{
    return dragEndY;
}

bool InputHandler::haveValidMousePos()
{
    return validMousePos;
}

std::map<std::string, std::string> InputHandler::getShortcutHelpList()
{
    std::map<std::string, std::string> output;
    std::map<int, std::string>::const_iterator di;
    for ( di = docstr.begin(); di != docstr.end(); di++ )
    {
        std::string line = htos( di->first );
        output.insert(
                std::pair<std::string, std::string>( line, di->second ) );
    }
    return output;
}
