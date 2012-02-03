/*
 * @file Frame.cpp
 *
 * Defines the GUI frame for the main video display window, as well as the side
 * list window.
 *
 * Created on: Jul 13, 2010
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

#include "Frame.h"
#include "GLCanvas.h"
#include "InputHandler.h"
#include "VideoInfoDialog.h"
#include "SessionTreeControl.h"
#include "ObjectManager.h"
#include "InputHandler.h"
#include "gravUtil.h"
#include "SideFrame.h"

int Frame::toggleRunwayID = wxNewId();
int Frame::toggleVCCID = wxNewId();
int Frame::toggleSideFrameID = wxNewId();
int Frame::toggleAutomaticID = wxNewId();

BEGIN_EVENT_TABLE(Frame, wxFrame)
EVT_CLOSE(Frame::OnCloseWindow)
// for right click -> properties in main window
EVT_MENU(InputHandler::propertyID, Frame::spawnPropertyWindow)
EVT_MENU(wxID_EXIT, Frame::OnExit)
EVT_MENU(wxID_ABOUT, Frame::OnAbout)
EVT_MENU(wxID_HELP_COMMANDS, Frame::OnKeyboardShortcuts)
EVT_MENU(toggleRunwayID, Frame::toggleRunwayEvent)
EVT_MENU(toggleVCCID, Frame::toggleVCCEvent)
EVT_MENU(toggleSideFrameID, Frame::toggleSideFrameEvent)
EVT_MENU(toggleAutomaticID, Frame::toggleAutomaticEvent)
EVT_MENU_OPEN(Frame::OnMenuOpen)
END_EVENT_TABLE();

Frame::Frame( wxWindow* parent, wxWindowID id, const wxString& title ) :
                wxFrame( parent, id, title, wxDefaultPosition, wxDefaultSize ),
                input( NULL )
{
    setupMenuBar();
}

Frame::Frame( wxWindow* parent, wxWindowID id, const wxString& title,
                const wxPoint& pos, const wxSize& size ) :
                wxFrame( parent, id, title, pos, size ),
                input( NULL )
{
    setupMenuBar();
}

void Frame::setObjectManager( ObjectManager* o )
{
    objectMan = o;
}

void Frame::setInputHandler( InputHandler* i )
{
    input = i;
}

void Frame::spawnPropertyWindow( wxCommandEvent& evt )
{
    for ( unsigned int i = 0; i < objectMan->getSelectedObjects()->size(); i++ )
    {
        VideoInfoDialog* dialog = new VideoInfoDialog( this,
                (*objectMan->getSelectedObjects())[i] );
        dialog->Show();
    }
}

void Frame::OnCloseWindow( wxCloseEvent& evt )
{
    // if the event can't be vetoed (ie, force close) then force close
    if ( !evt.CanVeto() )
    {
        cleanupAndDestroy();
    }
    else
    {
        // otherwise show "really quit" dialog, close window if OK clicked
        wxMessageDialog* exitDialog = new wxMessageDialog( this,
                _("Really quit?"), _("grav"), wxOK | wxCANCEL );
        int result = exitDialog->ShowModal();
        exitDialog->Destroy();

        switch( result )
        {
        case wxID_OK:
            cleanupAndDestroy();
            break;
        case wxID_CANCEL:
        default:
            evt.Veto();
            break;
        }
    }
}

void Frame::OnExit( wxCommandEvent& evt )
{
    Close();
}

void Frame::OnAbout( wxCommandEvent& evt )
{
    std::string about = "grav " + gravUtil::getVersionString() + "\n"
              "Copyright (C) 2011 Rochester Institute of Technology\n"
              "Authored by Andrew Ford with contributions from Ralph Bean.\n"
              "grav is free software, released under the GNU GPL. "
              "See COPYING for details.\n\n"
              "This software uses libraries from the FFmpeg project under the "
              "GPLv3.\n\n"
              "Earth imagery by NASA from the Visible Earth project.\n"
              "See http://visibleearth.nasa.gov/view_detail.php?id=2430 "
              "for more info.";

    wxMessageBox( wxString( about.c_str(), wxConvUTF8 ) );
}

void Frame::OnKeyboardShortcuts( wxCommandEvent& evt )
{
    wxDialog* helpDialog = new wxDialog( this, wxID_ANY,
            _("Keyboard Shortcuts") );
    helpDialog->SetSize( wxSize( 500, 500 ) );
    // note that size, i think, is only relevant as a minimum, since the sizer
    // will resize the window to the proper size

    wxStaticText* keyText = new wxStaticText( helpDialog, wxID_ANY, _("") );
    wxStaticText* helpText = new wxStaticText( helpDialog, wxID_ANY, _("") );
    std::string keyTextStd, helpTextStd;

    std::map<std::string, std::string> keyMap = input->getShortcutHelpList();
    std::map<std::string, std::string>::iterator i;
    for ( i = keyMap.begin(); i != keyMap.end(); ++i )
    {
        keyTextStd += i->first;
        helpTextStd += i->second;

        std::map<std::string, std::string>::iterator check = i;
        if ( ++check != keyMap.end() )
        {
            keyTextStd += "\n";
            helpTextStd += "\n";
        }
    }

    keyText->SetLabel( wxString( keyTextStd.c_str(), wxConvUTF8 ) );
    helpText->SetLabel( wxString( helpTextStd.c_str(), wxConvUTF8 ) );

    wxBoxSizer* textSizer = new wxBoxSizer( wxHORIZONTAL );
    textSizer->Add( keyText, wxSizerFlags(0).Align(0).Border( wxALL, 10 ) );
    textSizer->Add( helpText, wxSizerFlags(0).Align(0).Border( wxALL, 10 ) );

    helpDialog->SetSizer( textSizer );
    textSizer->SetSizeHints( helpDialog );

    helpDialog->ShowModal();
    helpDialog->Destroy();
}

void Frame::OnMenuOpen( wxMenuEvent& evt )
{
    wxMenu* menu = evt.GetMenu();
    wxMenuItemList list = menu->GetMenuItems();
    wxMenuItemList::iterator i;
    for ( i = list.begin(); i != list.end(); ++i )
    {
        if ( (*i)->GetId() == toggleRunwayID )
        {
            (*i)->Check( objectMan->usingRunway() );
        }
        else if ( (*i)->GetId() == toggleVCCID )
        {
            bool showable = objectMan->isVenueClientControllerShowable();
            (*i)->Enable( showable );
            if ( showable )
            {
                (*i)->Check( objectMan->isVenueClientControllerShown() );
            }
        }
        else if ( (*i)->GetId() == toggleSideFrameID )
        {
            SideFrame* sf = findChildSideFrame();
            if ( sf != NULL )
            {
                (*i)->Check( sf->IsShown() );
            }
        }
        else if ( (*i)->GetId() == toggleAutomaticID )
        {
            (*i)->Check( objectMan->usingAutoFocusRotate() );
        }
    }
}

void Frame::setupMenuBar()
{
    wxMenu *fileMenu = new wxMenu();
    fileMenu->AppendSeparator();
    fileMenu->Append( wxID_EXIT, _("Quit") );

    wxMenu *viewMenu = new wxMenu();
    viewMenu->AppendCheckItem( toggleRunwayID, _("Runway") );
    viewMenu->AppendCheckItem( toggleVCCID, _("Venue Client Controller") );
    viewMenu->AppendSeparator();
    viewMenu->AppendCheckItem( toggleSideFrameID, _("Groups/Sessions Window") );
    viewMenu->AppendSeparator();
    viewMenu->AppendCheckItem( toggleAutomaticID, _("Automatic Mode") );

    wxMenu *helpMenu = new wxMenu();
    helpMenu->Append( wxID_HELP_COMMANDS, _("Keyboard shortcuts...") );
    helpMenu->AppendSeparator();
    helpMenu->Append( wxID_ABOUT, _("About...") );

    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append( fileMenu, _("File") );
    menuBar->Append( viewMenu, _("View") );
    menuBar->Append( helpMenu, _("Help") );
    SetMenuBar( menuBar ) ;
}

void Frame::cleanupAndDestroy()
{
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

SideFrame* Frame::findChildSideFrame()
{
    wxWindowList children = GetChildren();
    wxWindowList::iterator i = children.begin();
    for ( ; i != children.end(); ++i )
    {
        SideFrame* sf = dynamic_cast<SideFrame*>( (*i) );
        if ( sf )
        {
            return sf;
        }
    }
    gravUtil::logWarning( "Frame::findChildSideFrame: could not find side frame"
            " child of this window\n" );
    return NULL;
}

void Frame::toggleRunwayEvent( wxCommandEvent& evt )
{
    objectMan->setRunwayUsage( !objectMan->usingRunway() );
    objectMan->clearSelected();
}

void Frame::toggleVCCEvent( wxCommandEvent& evt )
{
    objectMan->toggleShowVenueClientController();
}

void Frame::toggleSideFrameEvent( wxCommandEvent& evt )
{
    SideFrame* sf = findChildSideFrame();
    if ( sf != NULL )
    {
        sf->Show( !sf->IsShown() );
    }
}

void Frame::toggleAutomaticEvent( wxCommandEvent& evt )
{
    objectMan->setAutoFocusRotate( !objectMan->usingAutoFocusRotate() );
    objectMan->resetAutoCounter();
}
