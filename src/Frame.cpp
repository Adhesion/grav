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
EVT_MENU(wxID_EXIT, Frame::OnExit)
EVT_MENU(wxID_ABOUT, Frame::OnAbout)
END_EVENT_TABLE()

Frame::Frame( wxWindow* parent, wxWindowID id, const wxString& title ) :
                wxFrame( parent, id, title, wxDefaultPosition, wxDefaultSize )
{
    setupMenuBar();
}

Frame::Frame( wxWindow* parent, wxWindowID id, const wxString& title,
                const wxPoint& pos, const wxSize& size ) :
                wxFrame( parent, id, title, pos, size )
{
    setupMenuBar();
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
    // if the event can't be vetoed (ie, force close) then force close
    if ( !evt.CanVeto() )
    {
        cleanup();
        Destroy();
    }

    // otherwise show "really quit" dialog, close window if OK clicked
    wxMessageDialog* exitDialog = new wxMessageDialog( this, _("Really quit?"),
            _("grav"), wxOK | wxCANCEL );
    int result = exitDialog->ShowModal();
    exitDialog->Destroy();

    switch( result )
    {
    case wxID_OK:
        cleanup();
        Destroy();
        break;
    case wxID_CANCEL:
    default:
        evt.Veto();
        break;
    }
}

void Frame::OnExit( wxCommandEvent& evt )
{
    Close();
}

void Frame::OnAbout( wxCommandEvent& evt )
{
    wxMessageBox(
            _("grav (C) 2011 Rochester Institute of Technology\n"
              "grav is free software, released under the GNU GPL. "
              "See COPYING for details.\n\n"
              "This software uses libraries from the FFmpeg project under the "
              "GPLv3.") );
}

void Frame::setupMenuBar()
{
    wxMenu *fileMenu = new wxMenu();
    fileMenu->AppendSeparator();
    fileMenu->Append( wxID_EXIT, _("Quit") );

    wxMenu *helpMenu = new wxMenu();
    helpMenu->Append( wxID_HELP_COMMANDS, _("Keyboard shortcuts...") );
    helpMenu->AppendSeparator();
    helpMenu->Append( wxID_ABOUT, _("About...") );

    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append( fileMenu, _("File") );
    menuBar->Append( helpMenu, _("Help") );
    SetMenuBar( menuBar ) ;
}

void Frame::cleanup()
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
