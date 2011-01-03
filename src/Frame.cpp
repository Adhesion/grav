/**
 * @file Frame.cpp
 *
 * Defines the GUI frame for the main video display window, as well as the side
 * list window.
 *
 * Created on: Jul 13, 2010
 * @author Andrew Ford
 */

#include "Frame.h"
#include "GLCanvas.h"
#include "InputHandler.h"
#include "VideoInfoDialog.h"
#include "SessionTreeControl.h"
#include "gravManager.h"

BEGIN_EVENT_TABLE(Frame, wxFrame)
EVT_CLOSE(Frame::OnCloseWindow)
// for right click -> properties in main window
EVT_MENU(InputHandler::propertyID, Frame::spawnPropertyWindow)
// for menubar on side window, temporarily disabled
/*EVT_MENU(SessionTreeControl::addVideoID,
         SessionTreeControl::addVideoSessionEvent)
EVT_MENU(SessionTreeControl::addAudioID,
         SessionTreeControl::addAudioSessionEvent)
EVT_MENU(SessionTreeControl::rotateID,
         SessionTreeControl::rotateEvent)*/
END_EVENT_TABLE()

Frame::Frame( wxWindow* parent, wxWindowID id, const wxString& title ) :
                wxFrame( parent, id, title, wxDefaultPosition, wxDefaultSize )
{

}

Frame::Frame( wxWindow* parent, wxWindowID id, const wxString& title,
                const wxPoint& pos, const wxSize& size ) :
                wxFrame( parent, id, title, pos, size )
{

}

void Frame::setSourceManager( gravManager* g )
{
    grav = g;
}

void Frame::spawnPropertyWindow( wxCommandEvent& evt )
{
    for ( unsigned int i = 0; i < grav->getSelectedObjects()->size(); i++ )
    {
        VideoInfoDialog* dialog = new VideoInfoDialog( this,
                (*grav->getSelectedObjects())[i] );
        dialog->Show();
    }
}

void Frame::OnCloseWindow( wxCloseEvent& evt )
{
    // can potentially add a "really quit?" dialog here
    //printf( "Frame::OnCloseWindow (%s)\n", (char*)(GetName().char_str()) );

    // Note that destructors for children will automatically be called,
    // including glcanvas (which has the timer stop) and tree control.
    //
    // BUT: stop the timer right here if we have a glcanvas, since the timer
    // firing might get in the way of the various deconstructors in the wx
    // main loop and cause weird situations like OnExit never getting called
    // and even the children of this very frame not getting destroyed.
    //
    // Stopping the timer multiple times might be overkill but has no adverse
    // effects as far as I can tell.
    wxWindowList children = GetChildren();
    wxWindowList::iterator i = children.begin();
    for ( ; i != children.end(); ++i )
    {
        GLCanvas* canvas = dynamic_cast<GLCanvas*>( (*i) );
        if ( canvas )
        {
            canvas->stopTimer();
        }
    }

    Destroy();
}
