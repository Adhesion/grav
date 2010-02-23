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
#include "glutVideo.h"
#include "Earth.h"

#include <VPMedia/random_helper.h>

BEGIN_EVENT_TABLE(InputHandler, wxEvtHandler)
EVT_KEY_DOWN(InputHandler::wxKeyDown)
EVT_CHAR(InputHandler::wxCharEvt)
EVT_MOTION(InputHandler::wxMouseMove)
EVT_LEFT_DOWN(InputHandler::wxMouseLDown)
EVT_LEFT_UP(InputHandler::wxMouseLUp)
END_EVENT_TABLE()

InputHandler::InputHandler( Earth* e, gravManager* g )
    : earth( e ), grav( g )
{
    tempSelectedObjects = new std::vector<RectangleBase*>();
    dragging = false;
    clickedInside = false;
    leftButtonHeld = false;
    ctrlHeld = false;
    shiftHeld = false;
}

InputHandler::~InputHandler()
{ 
    delete tempSelectedObjects;
    // all other pointers are owned by the main class
}

void InputHandler::wxKeyDown( wxKeyEvent& evt )
{
    if ( evt.GetModifiers() == wxMOD_SHIFT )
        shiftHeld = true;
    else
        shiftHeld = false;
    evt.Skip(); // so now the char event can grab this
}

void InputHandler::wxCharEvt( wxKeyEvent& evt )
{
    // TODO: replace 0,0 with stored mouse pos
    processKeyboard( (unsigned char)evt.GetKeyCode(), 0, 0 );
}

void InputHandler::wxMouseMove( wxMouseEvent& evt )
{
    //printf( "mousemove\n" );
    if ( leftButtonHeld )
        mouseLeftHeldMove( evt.GetPosition().x, evt.GetPosition().y );
}

void InputHandler::wxMouseLDown( wxMouseEvent& evt )
{
    //printf( "mouseldown\n" );
    leftClick( evt.GetPosition().x, evt.GetPosition().y );
    evt.Skip();
}

void InputHandler::wxMouseLUp( wxMouseEvent& evt )
{
    //printf( "mouselup\n" );
    leftRelease( evt.GetPosition().x, evt.GetPosition().y );
    evt.Skip();
}

void InputHandler::processKeyboard( unsigned char key, int x, int y )
{
    std::vector<VPMVideoBufferSink*>::iterator t;
    std::map<std::string,Group*>::iterator mapi;
    printf( "Char pressed is %c (%i)\n", key, key );
    printf( "x,y in processKeyboard is %i,%i\n", x, y );
    std::vector<VideoSource*>::const_iterator si;
    // how much to scale when doing -/+: flipped in the former case
    float scaleAmt = 0.25f;
    
    switch(key) {
    
        case 'k':
            printf( "current sources selected: %i\n", 
                        grav->getSelectedObjects()->size() );
            for ( unsigned int i = 0; i < grav->getSelectedObjects()->size();
                    i++ )
            {
                printf( "%s\n",
                    (*(grav->getSelectedObjects()))[i]->getName().c_str() );
            }
            break;
      
        case 't':
            printf( "rearranging groups...\n" );
            for ( unsigned int i = 0; i < grav->getSelectedObjects()->size();
                    i++ )
            {
                Group* g = dynamic_cast<Group*>(
                                (*(grav->getSelectedObjects()))[i] );
                if ( g != NULL )
                {
                    g->rearrange();
                }
            }
            break;
        
        case 'u':
            printf( "updating group names...\n" );
            mapi = grav->getSiteIDGroups()->begin();
            for ( ; mapi != grav->getSiteIDGroups()->end(); mapi++ )
            {
                mapi->second->updateName();
            }
            break;
        
        //case 'P':
        case 'p':
            grav->perimeterAllVideos();
            break;
        
        case 'R':
        case 'r':
            grav->retileVideos();
            break;
        
        case 'l':
            printf( "We currently have %i sources.\n",
                     grav->getSources()->size() );
            printf( "We currently have %i objects in drawnObjects.\n",
                     grav->getDrawnObjects()->size() );
    
            for ( si = grav->getSources()->begin();
                    si != grav->getSources()->end(); si++ )
            {
                printf( "name: %s\n", 
                    (*si)->getMetadata(
                        VPMSession::VPMSESSION_SDES_NAME).c_str() );
                printf( "cname: %s\n", 
                    (*si)->getMetadata(
                        VPMSession::VPMSESSION_SDES_CNAME).c_str() );
                printf( "loc: %s\n", 
                    (*si)->getMetadata(
                        VPMSession::VPMSESSION_SDES_LOC).c_str() );
                printf( "ssrc 0x%08x\n", (*si)->getssrc() );
                printf( "\tpos (world): %f,%f\n",
                        (*si)->getX(), (*si)->getY() );
                GLdouble scrX; GLdouble scrY; GLdouble scrZ;
                GLUtil::getInstance()->worldToScreen( (GLdouble)(*si)->getX(),
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
        
        case 'N':
        case 'n':
            for ( si = grav->getSources()->begin();
                    si != grav->getSources()->end(); si++ )
            {
                if ( shiftHeld || (*si)->isSelected() )
                {
                    (*si)->scaleNative();
                    if ( (*si)->isGrouped() )
                        (*si)->getGroup()->rearrange();
                }
            }
            break;
        
        case '0':
            for ( si = grav->getSources()->begin();
                    si != grav->getSources()->end(); si++ )
            {
                (*si)->move(0.0f,0.0f);
            }
            break;
        
        case 'G':
        case 'g':
            if ( grav->usingSiteIDGroups() )
            {
                grav->setSiteIDGrouping( false );
                grav->ungroupAll();
            }
            else
                grav->setSiteIDGrouping( true );
            break;
        
        case '-':
            scaleAmt *= -1.0f;
        case '+':
            for ( si = grav->getSources()->begin();
                    si != grav->getSources()->end(); si++ )
            {
                if ( (*si)->isSelected() )
                {
                    (*si)->setScale( (*si)->getScaleX()+(*si)->getScaleX()*scaleAmt,
                                     (*si)->getScaleY()+(*si)->getScaleY()*scaleAmt );
                }
            }
            break;
        
        case 'w':
            grav->setCamZ(grav->getCamZ()-1);
            break;
        case 's':
            grav->setCamZ(grav->getCamZ()+1);
            break;
        case 'a':
            grav->setCamX(grav->getCamX()-1);
            break;
        case 'd':
            grav->setCamX(grav->getCamX()+1);
            break;
        
        // u/d/l/r arrow keys, for WX
        case ':':
            earth->rotate( 0.0f, 0.0f, -2.0f );
            break;
        case '<':
            earth->rotate( 0.0f, 0.0f, 2.0f );
            break;
        case ';':
            earth->rotate( -2.0f, 0.0f, 0.0f );
            break;
        case '=':
            earth->rotate( 2.0f, 0.0f, 0.0f );
            break;
        
        // backspace: deselect videos
        case 8:
            grav->clearSelected();
            break;
        
        // space
        case 32:
            grav->addTestObject();
            break;
        
        // shift
        case WXK_SHIFT:
            printf( "shift held\n" );
            break;
        
        case 'Q':
        case 'q':
        case 27:
            exit(0);
            break;
    }
}

void InputHandler::processSpecialKey( int key, int x, int y )
{
    switch( key )
    {
        case GLUT_KEY_LEFT:
        case ':':
            earth->rotate( 0.0f, 0.0f, -2.0f );
            break;
        case GLUT_KEY_RIGHT:
        case '<':
            earth->rotate( 0.0f, 0.0f, 2.0f );
            break;
        case GLUT_KEY_UP:
        case ';':
            earth->rotate( -2.0f, 0.0f, 0.0f );
            break;
        case GLUT_KEY_DOWN:
        case '=':
            earth->rotate( 2.0f, 0.0f, 0.0f );
            break;
    }
}

void InputHandler::processMouse( int button, int state, int x, int y )
{
    special = glutGetModifiers();
    
    if ( state == GLUT_DOWN && button == GLUT_LEFT_BUTTON )
    {
        leftClick( x, y );
    }
    else if ( state == GLUT_UP )
    {
        leftRelease( x, y );
    }
}

void InputHandler::processActiveMotion( int x, int y )
{
    mouseLeftHeldMove( x, y );
}

void InputHandler::leftClick( int x, int y )
{
    // glut screen coords are y-flipped relative to GL screen coords
    //printf( "window height? %i\n", grav->getWindowHeight() );
    y = grav->getWindowHeight() - y;
    
    grav->setBoxSelectDrawing( false );
    
    // get world coords for current mouse pos
    GLUtil::getInstance()->screenToWorld( (GLdouble)x, (GLdouble)y, 0.990991f,
                            &mouseX, &mouseY, &mouseZ );
    
    printf( "mouse clicked at world %f,%f; screen %i,%i\n",
            mouseX, mouseY, x, y );
    
    // on click, any potential dragging afterwards must start here
    dragStartX = mouseX;
    dragStartY = mouseY;
    dragPrevX = mouseX;
    dragPrevY = mouseY;
    
    // if we didn't click on a video, and we're not holding down ctrl to
    // begin a multi selection, so move the selected video(s) to the
    // clicked position in empty space
    selectVideos();
    if ( !clickedInside && special != GLUT_ACTIVE_CTRL )
    {
        if ( !grav->getSelectedObjects()->empty() )
        {
            float movePosX = mouseX; float movePosY = mouseY;
            std::vector<RectangleBase*>::const_iterator sli;
            
            float avgX = 0.0f, avgY = 0.0f;
            int num = grav->getSelectedObjects()->size();
            
            if ( num == 1 )
            {
                (*(grav->getSelectedObjects()))[0]->move( movePosX, movePosY );
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
                    (*sli)->move( movePosX + ((*sli)->getX()-avgX),
                                   movePosY + ((*sli)->getY()-avgY) );
                    (*sli)->setSelect( false );
                }
            }
            grav->getSelectedObjects()->clear();
        }
    }
    // or if we did click on a video...
    else
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
        grav->getSelectedObjects()->push_back( (*tempSelectedObjects)[i] );
    tempSelectedObjects->clear();
    leftButtonHeld = false;
    
    // if we were doing drag movement, deselect all
    if ( dragging && grav->getHoldCounter() > 10 )
    {
        grav->clearSelected();
        dragging = false;
    }
}

void InputHandler::mouseLeftHeldMove( int x, int y )
{
    // glut screen coords are y-flipped relative to GL screen coords
    y = grav->getWindowHeight() - y;
    
    // get world coords for current mouse pos
    GLUtil::getInstance()->screenToWorld( (GLdouble)x, (GLdouble)y, 0.990991f,
                            &mouseX, &mouseY, &mouseZ );
    
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
            if ( !(*sli)->isGrouped() )
            {
                (*sli)->setPos( (dragEndX-dragPrevX) + (*sli)->getX(),
                                (dragEndY-dragPrevY) + (*sli)->getY() );
            }
            else if ( !(*sli)->getGroup()->isSelected() )
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
    
    grav->incrementHoldCounter();
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
            si != grav->getDrawnObjects()->rend(); si++ )
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
        
        bool intersect = (*si)->intersect( selectL, selectR,
                                        selectU, selectD );
        
        // for click-and-drag movement
        clickedInside = intersect && !leftButtonHeld;
        
        if ( intersect )
        {
            // clear the list of selected if we're clicking on a new video
            if ( !leftButtonHeld && !(*si)->isSelected() &&
                    special != GLUT_ACTIVE_CTRL )
                grav->clearSelected();
            
            videoSelected = true;
            
            if ( !(*si)->isSelected() )
            {
                (*si)->setSelect( true );
                // if we're doing a box selection, add it to the temp list
                if ( leftButtonHeld )
                    tempSelectedObjects->push_back( *si );
                else
                    grav->getSelectedObjects()->push_back( *si );
            }
            
            // take the selected video and put it at the end of the list so
            // it'll be rendered on top - but only if we just clicked on it
            if ( !leftButtonHeld )
            {   
                // since we can only delete a normal iterator (not a reverse
                // one) we have to calculate our current position
                std::vector<RectangleBase*>::iterator current =
                    grav->getDrawnObjects()->begin() - 1 + 
                    distance( si, grav->getDrawnObjects()->rend() );
                grav->moveToTop( current );
                
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
