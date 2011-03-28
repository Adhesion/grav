/**
 * @file InputHandler.h
 * Implementation of a class for processing keyboard & mouse input and managing
 * selection & movement of objects accordingly.
 * @author Andrew Ford
 */

#include "InputHandler.h"
#include "GLUtil.h"
#include "VideoSource.h"
#include "RectangleBase.h"
#include "Group.h"
#include "gravManager.h"
#include "Earth.h"
#include "Frame.h"
#include "Runway.h"

#include <VPMedia/random_helper.h>

/* For key formatting */
#include <sstream>
#include <iomanip>

BEGIN_EVENT_TABLE(InputHandler, wxEvtHandler)
EVT_KEY_DOWN(InputHandler::wxKeyDown)
EVT_CHAR(InputHandler::wxCharEvt)
EVT_MOTION(InputHandler::wxMouseMove)
EVT_LEFT_DOWN(InputHandler::wxMouseLDown)
EVT_LEFT_DCLICK(InputHandler::wxMouseLDClick)
EVT_LEFT_UP(InputHandler::wxMouseLUp)
EVT_RIGHT_DOWN(InputHandler::wxMouseRDown)
END_EVENT_TABLE()

int InputHandler::propertyID = wxNewId();

InputHandler::InputHandler( Earth* e, gravManager* g, Frame* f )
    : earth( e ), grav( g ), mainFrame( f )
{
    tempSelectedObjects = new std::vector<RectangleBase*>();
    dragging = false;
    clickedInside = false;
    leftButtonHeld = false;
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
    docstr[ktoh('\b')] = "Select none.";
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
    docstr[ktoh('N')] = "Scale selected objects to native size.";
    lookup[ktoh('N', wxMOD_SHIFT)] = &InputHandler::handleNativeScaleAll;
    docstr[ktoh('N', wxMOD_SHIFT)] = "Scale all objects to native size.";

    /* Different Layouts */
    lookup[ktoh('P')] = &InputHandler::handlePerimeterArrange;
    docstr[ktoh('P')] = "Arrange objects around the perimeter of the screen.";
    lookup[ktoh('R')] = &InputHandler::handleGridArrange;
    docstr[ktoh('R')] = "Arrange objects into a grid.";
    lookup[ktoh('F')] = &InputHandler::handleFocusArrange;
    docstr[ktoh('F')] = "Rearrange objects to focus on selected objects.";
    lookup[ktoh('A', wxMOD_ALT)] = &InputHandler::handleToggleAutoFocusRotate;
    docstr[ktoh('A', wxMOD_ALT)] = "Toggle 'automatic' mode.";

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
        unprintables[ktoh((unsigned char)127)] = "(delete)";
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
    if ( leftButtonHeld )
        mouseLeftHeldMove( evt.GetPosition().x, evt.GetPosition().y );
}

void InputHandler::wxMouseLDown( wxMouseEvent& evt )
{
    // TODO fix these? how to best handle mouse modifiers?
    if ( evt.CmdDown() )
        ctrlHeld = true;
    else
        ctrlHeld = false;
    //modifiers = evt.GetModifiers();

    leftClick( evt.GetPosition().x, evt.GetPosition().y );
    evt.Skip();
}

void InputHandler::wxMouseLUp( wxMouseEvent& evt )
{
    leftRelease( evt.GetPosition().x, evt.GetPosition().y );
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

    leftClick( evt.GetPosition().x, evt.GetPosition().y, true );
    evt.Skip();
}

void InputHandler::wxMouseRDown( wxMouseEvent& evt )
{
    if ( grav->getSelectedObjects()->size() > 0 )
    {
        wxMenu rightClickMenu;
        rightClickMenu.Append( propertyID, _("Properties") );
        mainFrame->PopupMenu( &rightClickMenu, evt.GetPosition() );
    }
}

void InputHandler::handlePrintSelected()
{
    printf( "current sources selected: %i\n",
            grav->getSelectedObjects()->size() );
    for ( unsigned int i = 0; i < grav->getSelectedObjects()->size(); i++ )
    {
        printf( "%s\n", (*(grav->getSelectedObjects()))[i]->getName().c_str() );
    }
}

void InputHandler::handleRearrangeGroups()
{
    printf( "rearranging groups...\n" );
    for ( unsigned int i = 0; i < grav->getSelectedObjects()->size(); i++ )
    {
        RectangleBase* temp = (*(grav->getSelectedObjects()))[i];
        if ( temp->isGroup() )
        {
            Group* g = (Group*)temp;
            g->rearrange();
        }
    }
}

void InputHandler::handleUpdateGroupNames()
{
    printf( "updating group names...\n" );
    std::map<std::string,Group*>::iterator mapi;
    mapi = grav->getSiteIDGroups()->begin();
    for ( ; mapi != grav->getSiteIDGroups()->end(); mapi++ )
    {
        mapi->second->updateName();
    }
}

void InputHandler::handlePerimeterArrange()
{
    std::map<std::string, std::vector<RectangleBase*> > data;
    data["objects"] = grav->getMovableObjects();
    layouts.arrange(
        "perimeter", grav->getScreenRect(), grav->getEarthRect(), data );
}

void InputHandler::handleGridArrange()
{
    std::map<std::string, std::vector<RectangleBase*> > data;
    data["objects"] = grav->getMovableObjects();
    layouts.arrange( "grid", grav->getScreenRect(), RectangleBase(), data );
}

void InputHandler::handleFocusArrange()
{
    if ( grav->getSelectedObjects()->size() > 0 )
    {
        std::map<std::string, std::vector<RectangleBase*> > data;
        data["outers"] = grav->getUnselectedObjects();
        data["inners"] = *(grav->getSelectedObjects());
        layouts.arrange( "aspectFocus", grav->getScreenRect(), RectangleBase(),
                            data );
    }
}

void InputHandler::handleFullscreenSelectedSingle()
{
    if ( grav->getSelectedObjects()->size() == 1 )
    {
        (*grav->getSelectedObjects())[0]->fillToRect(grav->getScreenRect());
    }
}

void InputHandler::handleFullerFullscreenSelectedSingle()
{
    if ( grav->getSelectedObjects()->size() == 1 )
    {
        (*grav->getSelectedObjects())[0]->fillToRect(
                grav->getScreenRect( true ), true );
    }
}

void InputHandler::handleRunwayToggle()
{
    grav->setRunwayUsage( !grav->usingRunway() );
    grav->clearSelected();
}

void InputHandler::handleInvertSelection()
{
    std::vector<RectangleBase*> movableObjects = grav->getMovableObjects();
    for ( unsigned int i = 0; i < movableObjects.size(); i++ )
    {
        RectangleBase* obj = movableObjects[i];
        obj->setSelect( !obj->isSelected() );
        if ( obj->isSelected() )
        {
            grav->getSelectedObjects()->push_back( obj );
        }
        else
        {
            std::vector<RectangleBase*>::iterator it =
                    grav->getSelectedObjects()->begin();
            while ( (*it) != obj ) it++;
            grav->getSelectedObjects()->erase( it );
        }
    }
}

void InputHandler::handleHelp()
{
    std::vector<std::string>::const_iterator i;
    std::vector<std::string> helpList = getShortcutHelpList();
    printf( "List of mapped keys:\n" );
    for ( i = helpList.begin(); i != helpList.end(); ++i )
    {
        printf( "%s\n", (*i).c_str() );
    }
}

void InputHandler::handleInformation()
{
    std::vector<VideoSource*>::const_iterator si;
    printf( "We currently have %i sources.\n",
             grav->getSources()->size() );
    printf( "We currently have %i objects in drawnObjects.\n",
             grav->getDrawnObjects()->size() );
    printf( "Screen size is %f x %f\n",
            grav->getScreenRect().getWidth(),
            grav->getScreenRect().getHeight() );

    for ( si = grav->getSources()->begin();
            si != grav->getSources()->end(); si++ )
    {
        printf( "name: %s\n",
            (*si)->getMetadata(
                VPMSession::VPMSESSION_SDES_NAME).c_str() );
        printf( "stored name: %s\n", (*si)->getName().c_str() );
        printf( "cname: %s\n",
            (*si)->getMetadata(
                VPMSession::VPMSESSION_SDES_CNAME).c_str() );
        printf( "stored altname: %s\n", (*si)->getAltName().c_str() );
        printf( "loc: %s\n",
            (*si)->getMetadata(
                VPMSession::VPMSESSION_SDES_LOC).c_str() );
        printf( "ssrc 0x%08x (%d)\n", (*si)->getssrc(),
                    (*si)->getssrc() );
        printf( "\tpos (world): %f,%f\n",
                (*si)->getX(), (*si)->getY() );
        GLdouble scrX; GLdouble scrY; GLdouble scrZ;
        GLUtil::getInstance()->worldToScreen( (GLdouble)(*si)->getX(),
                        (GLdouble)(*si)->getY(),
                        (GLdouble)(*si)->getZ(),
                        &scrX, &scrY, &scrZ);
        printf( "\tpos (screen): %f,%f,%f\n", scrX, scrY, scrZ );
        printf( "\tis grouped? %i\n", (*si)->isGrouped() );

        printf( "\tDescription of codec: %s\n",
                    (*si)->getPayloadDesc() );
        printf( "\tis muted? %i\n", (*si)->isMuted() );

        printf( "\tSize: %f x %f\n", (*si)->getWidth(),
                    (*si)->getHeight() );
        printf( "\tText size: %f x %f\n", (*si)->getTextWidth(),
                    (*si)->getTextHeight() );
    }
    printf( "DrawnObjects:\n" );
    for ( unsigned int i = 0; i < grav->getDrawnObjects()->size(); i++ )
    {
        RectangleBase* temp = (*(grav->getDrawnObjects()))[i];
        printf( "%s (%fx%f)\n", temp->getName().c_str(),
                temp->getDestWidth(), temp->getDestHeight() );
    }
}

void InputHandler::handleToggleGroupLocks()
{
    for ( unsigned int i = 0; i < grav->getSelectedObjects()->size(); i++ )
    {
        RectangleBase* temp = (*(grav->getSelectedObjects()))[i];
        if ( temp->isGroup() )
        {
            Group* g = (Group*)temp;
            printf( "InputHandler::l:group %s locked? %i\n",
                        g->getName().c_str(), g->isLocked() );
            g->changeLock();
            printf( "after: %i\n", g->isLocked() );
        }
    }
}

void InputHandler::handleMuteSelected()
{
    std::vector<VideoSource*>::const_iterator si;
    for ( si = grav->getSources()->begin();
                    si != grav->getSources()->end(); ++si )
    {
        if ( (*si)->isSelected() )
        {
            (*si)->toggleMute();
            // add it to the runway if we're using it, it's visible, etc.
            if ( !(*si)->isGrouped() && (*si)->isMuted() &&
                    grav->usingRunway() &&
                    grav->getRunway()->getRendering() )
            {
                grav->getRunway()->add( (*si) );
                (*si)->setSelect( false );
            }
        }
    }
    grav->clearSelected();
}

void InputHandler::handleRandomTest()
{
    printf( "random32: %i\n", random32() );
    printf( "random32max: %i\n", random32_max() );
}

void InputHandler::handleNativeScaleAll()
{
    std::vector<VideoSource*>::const_iterator si;
    for ( si = grav->getSources()->begin();
            si != grav->getSources()->end(); ++si )
    {
        (*si)->scaleNative();
        if ( (*si)->isGrouped() )
            (*si)->getGroup()->rearrange();
    }
}

void InputHandler::handleNativeScaleSelected()
{
    std::vector<VideoSource*>::const_iterator si;
    for ( si = grav->getSources()->begin();
            si != grav->getSources()->end(); ++si )
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
    for ( si = grav->getSources()->begin();
            si != grav->getSources()->end(); ++si )
    {
        (*si)->move( 0.0f, 0.0f );
    }
}

void InputHandler::handleToggleSiteGrouping()
{
    // TODO -- condense this with a bang
    if ( grav->usingSiteIDGroups() )
    {
        grav->setSiteIDGrouping( false );
        grav->ungroupAll();
    }
    else
    {
        grav->setSiteIDGrouping( true );
    }
}

void InputHandler::handleToggleShowVenueClientController()
{
    grav->toggleShowVenueClientController();
}

void InputHandler::handleToggleRenderingSelected()
{
    for ( unsigned int i = 0; i < grav->getSelectedObjects()->size();
                    i++ )
    {
        RectangleBase* temp = (*(grav->getSelectedObjects()))[i];
        VideoSource* tempVS = dynamic_cast<VideoSource*>( temp );
        if ( tempVS != NULL )
        {
            tempVS->setRendering( !tempVS->getRendering() );
        }
    }
}

void InputHandler::handleZoomout()
{
        grav->setCamZ(grav->getCamZ()+1);
}

void InputHandler::handleZoomin()
{
        grav->setCamZ(grav->getCamZ()-1);
}

void InputHandler::handleStrafeLeft()
{
        grav->setCamX(grav->getCamX()-1);
}

void InputHandler::handleStrafeRight()
{
        grav->setCamX(grav->getCamX()+1);
}

void InputHandler::handleToggleAutoFocusRotate()
{
    grav->setAutoFocusRotate( !grav->usingAutoFocusRotate() );
    grav->resetAutoCounter();
}

void InputHandler::handleSelectAll()
{
    std::vector<RectangleBase*> movableObjects = grav->getMovableObjects();
    grav->clearSelected();
    for ( unsigned int i = 0; i < movableObjects.size(); i++ )
    {
        movableObjects[i]->setSelect( true );
        grav->getSelectedObjects()->push_back( movableObjects[i] );
    }
}

void InputHandler::handleToggleGraphicsDebug()
{
    grav->setGraphicsDebugMode( !grav->getGraphicsDebugMode() );
}

void InputHandler::handleDownscaleSelected()
{
    float scaleAmt = 0.25f;
    grav->scaleSelectedObjects( scaleAmt * -1.0f );
}

void InputHandler::handleUpscaleSelected()
{
    float scaleAmt = 0.25f;
    grav->scaleSelectedObjects( scaleAmt );
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
    grav->clearSelected();
}

void InputHandler::handleAddTestObject()
{
    grav->addTestObject();
}

void InputHandler::handleTryDeleteObject()
{
    std::vector<RectangleBase*> movableObjects = grav->getMovableObjects();
    for ( unsigned int i = 0; i < movableObjects.size(); i++ )
    {
        RectangleBase* obj = movableObjects[i];
        if ( obj->isSelected() )
            grav->tryDeleteObject( obj );
    }
}

/* Key To Hash */
int InputHandler::ktoh( unsigned char key ) { return ktoh(key, 0); }
int InputHandler::ktoh( unsigned char key, int _modifiers ) {
    return ( ((int)key)  << 4 ) | _modifiers;
}
/* Hash To Key */
unsigned char InputHandler::htok( int hash ) {
    return (unsigned char)(hash >> 4);
}
/* Hash to String representation */
std::string InputHandler::htos( int hash ) {
    char key = htok( hash );
    std::map<char, std::string>::iterator upIter = unprintables.find(key);
    std::ostringstream sstr;
    std::string _mods;

    std::string shi = (hash & wxMOD_SHIFT) ? "shift + " : "";
    std::string alt = (hash & wxMOD_ALT) ? "alt + " : "";
    std::string cmd = (hash & wxMOD_CMD) ? "ctrl + " : "";

    sstr << shi << alt << cmd;
    _mods = sstr.str();
    sstr.str("");
    sstr << std::setw(25) << _mods;

    if ( upIter != unprintables.end() )
        sstr << unprintables[key];
    else
        sstr << key;

    return sstr.str();
}

void InputHandler::processKeyboard( int keyCode, int x, int y )
{
    std::vector<VPMVideoBufferSink*>::iterator t;
    unsigned char key = (unsigned char)keyCode;
    int hash = ktoh(key, this->modifiers);
    /*printf( "Char pressed is %c (%i)\n", key, key );
    printf( "keycode is %i\n", keyCode );
    printf( "x,y in processKeyboard is %i,%i\n", x, y );
    printf( "is shift (only) held? %i\n", modifiers == wxMOD_SHIFT );
    printf( "is ctrl (only) held? %i\n", modifiers == wxMOD_CMD );
    printf( "is alt (only) held? %i\n", modifiers == wxMOD_ALT );*/
    // how much to scale when doing -/+: flipped in the former case

    // Lookup the pressed key in our map of method pointers
    std::map<int, MFP>::iterator lookupIter;
    lookupIter = lookup.find(hash);
    if( lookupIter != lookup.end() )
        (this->*(lookup[hash]))();
    else
        printf( "No handler for registered for key (code is %i):\n%s\n",
                keyCode, htos(hash).c_str());

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

void InputHandler::leftClick( int x, int y, bool doubleClick )
{
    // glut screen coords are y-flipped relative to GL screen coords
    //printf( "window height? %i\n", grav->getWindowHeight() );
    y = grav->getWindowHeight() - y;

    grav->setBoxSelectDrawing( false );

    // old method for getting world coords for current mouse pos
    //GLUtil::getInstance()->screenToWorld( (GLdouble)x, (GLdouble)y, 0.99087065,
    //                            &mouseX, &mouseY, &mouseZ );
    //printf( "leftClick::old method got %f,%f,%f\n", mouseX, mouseY, mouseZ );

    // ray intersect-based click method
    /*Point nearScreen( x, y, 0.0f );
    Point farScreen( x, y, 0.5f );
    Point near, far;
    GLUtil::getInstance()->screenToWorld( nearScreen, near );
    GLUtil::getInstance()->screenToWorld( farScreen, far );

    Ray r;
    r.location = near;
    r.direction = far - near;
    Point intersect;
    bool rayRetVal = grav->getScreenRect().findRayIntersect( r, intersect );
    if ( !rayRetVal )
        return;*/

    Point intersect;
    bool ret = GLUtil::getInstance()->screenToRectIntersect( x, y,
                                                          grav->getScreenRect(),
                                                          intersect );
    if ( !ret )
        return;
    mouseX = intersect.getX();
    mouseY = intersect.getY();

    // on click, any potential dragging afterwards must start here
    dragStartX = mouseX;
    dragStartY = mouseY;
    dragPrevX = mouseX;
    dragPrevY = mouseY;

    // if we didn't click on a video, and we're not holding down ctrl to
    // begin a multi selection, so move the selected video(s) to the
    // clicked position in empty space
    selectVideos();
    if ( !clickedInside && !ctrlHeld )
    {
        if ( !grav->getSelectedObjects()->empty() )
        {
            float movePosX = mouseX; float movePosY = mouseY;
            std::vector<RectangleBase*>::const_iterator sli;

            float avgX = 0.0f, avgY = 0.0f;
            int num = grav->getSelectedObjects()->size();

            if ( num == 1 )
            {
                if ( (*(grav->getSelectedObjects()))[0]->isUserMovable() )
                {
                    (*(grav->getSelectedObjects()))[0]->move( movePosX,
                                                                movePosY );
                }
                (*(grav->getSelectedObjects()))[0]->setSelect( false );
            }
            // if moving >1, center the videos around the click point
            // we need to find the average pos beforehand
            else
            {
                // average the vals
                for ( sli = grav->getSelectedObjects()->begin();
                       sli != grav->getSelectedObjects()->end();
                       sli++ )
                {
                    avgX += (*sli)->getX();
                    avgY += (*sli)->getY();
                }
                avgX = avgX / num;
                avgY = avgY / num;

                // move and set them to be unselected
                for ( sli = grav->getSelectedObjects()->begin();
                       sli != grav->getSelectedObjects()->end();
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
            grav->getSelectedObjects()->clear();
        }
    }
    // or if we did click on a video...
    else if ( doubleClick )
    {
        if ( grav->getSelectedObjects()->size() == 1 )
        {
            (*grav->getSelectedObjects())[0]->doubleClickAction();
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

void InputHandler::leftRelease( int x, int y )
{
    // shift the temporary list of selected objects to the main list to
    // allow for multiple box selection
    for ( unsigned int i = 0; i < tempSelectedObjects->size(); i++ )
    {
        grav->getSelectedObjects()->push_back( (*tempSelectedObjects)[i] );
    }
    tempSelectedObjects->clear();
    leftButtonHeld = false;

    // if we were doing drag movement, deselect all
    // the getholdcounter check is to prevent user from losing a click selection
    // if they accidentally drag it a tiny bit
    if ( dragging && grav->getHoldCounter() > 5 )
    {
        grav->clearSelected();
    }

    // since there is no other way to drag (at the moment) without holding down
    // the mouse, we're necessarily not dragging anymore
    // (dragging in this case refers to dragging an object, not a select box)
    // putting this here rather than in the above if statement prevents box
    // selection from failing after an accidental small drag
    dragging = false;
}

void InputHandler::mouseLeftHeldMove( int x, int y )
{
    // glut screen coords are y-flipped relative to GL screen coords
    y = grav->getWindowHeight() - y;

    // old method for getting world coords for current mouse pos
    //GLUtil::getInstance()->screenToWorld( (GLdouble)x, (GLdouble)y, 0.990991f,
    //                        &mouseX, &mouseY, &mouseZ );

    // ray intersect-based click method
    /*Point nearScreen( x, y, 0.0f );
    Point farScreen( x, y, 0.5f );
    Point near, far;
    GLUtil::getInstance()->screenToWorld( nearScreen, near );
    GLUtil::getInstance()->screenToWorld( farScreen, far );

    Ray r;
    r.location = near;
    r.direction = far - near;
    Point intersect;
    bool rayRetVal = grav->getScreenRect().findRayIntersect( r, intersect );
    if ( !rayRetVal )
        return;*/

    Point intersect;
    bool ret = GLUtil::getInstance()->screenToRectIntersect( x, y,
                                                          grav->getScreenRect(),
                                                          intersect );
    if ( !ret )
        return;
    mouseX = intersect.getX();
    mouseY = intersect.getY();

    dragEndX = mouseX;
    dragEndY = mouseY;

    // set new position, when doing click-and-drag movement
    if ( clickedInside )
    {
        std::vector<RectangleBase*>::reverse_iterator sli;
        for ( sli = grav->getSelectedObjects()->rbegin();
                sli != grav->getSelectedObjects()->rend();
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
        grav->setBoxSelectDrawing( false );
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
        grav->setBoxSelectDrawing( true );
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

    for ( si = grav->getDrawnObjects()->rbegin();
            si != grav->getDrawnObjects()->rend(); ++si )
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
                grav->clearSelected();

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
                    grav->getSelectedObjects()->push_back( temp );
            }
            // if what we just clicked on is selected and we're holding ctrl,
            // deselect
            else if ( ctrlHeld && !leftButtonHeld )
            {
                temp->setSelect( false );
                std::vector<RectangleBase*>::iterator sli =
                                    grav->getSelectedObjects()->begin();
                while ( (*sli) != temp )
                    sli++;
                grav->getSelectedObjects()->erase( sli );
            }

            // take the selected video and put it at the end of the list so
            // it'll be rendered on top - but only if we just clicked on it
            if ( !leftButtonHeld )
            {
                //printf( "putting selected video to end\n" );
                // since we can only delete a normal iterator (not a reverse
                // one) we have to calculate our current position
                /*std::vector<RectangleBase*>::iterator current =
                    grav->getDrawnObjects()->begin() - 1 +
                    distance( si, grav->getDrawnObjects()->rend() );
                grav->moveToTop( current );*/
                grav->moveToTop( temp );

                break; // so we only select one video per click
                       // when single-clicking
            }
        }
        else
        {
            //printf( "InputHandler::selectVideos: did not intersect\n" );
        }
    }

    return videoSelected;
}

bool InputHandler::isLeftButtonHeld()
{
    return leftButtonHeld;
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

std::vector<std::string> InputHandler::getShortcutHelpList()
{
    std::vector<std::string> output;
    std::map<int, std::string>::const_iterator di;
    for ( di = docstr.begin(); di != docstr.end(); di++ )
    {
        std::string line = htos( di->first ) + "\t" + di->second;
        output.push_back( line );
    }
    return output;
}
