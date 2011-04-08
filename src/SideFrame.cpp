/*
 * @file SideFrame.cpp
 *
 * Implementation of the side GUI window.
 *
 * Created on: Oct 12, 2010
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

#include "SideFrame.h"
#include "SessionTreeControl.h"

int SideFrame::addVideoID = wxNewId();
int SideFrame::addAudioID = wxNewId();
int SideFrame::rotateID = wxNewId();

SideFrame::SideFrame( wxWindow* parent, wxWindowID id, const wxString& title ) :
                wxFrame( parent, id, title, wxDefaultPosition, wxDefaultSize )
{
    setupMenuBar();
}

SideFrame::SideFrame( wxWindow* parent, wxWindowID id, const wxString& title,
                const wxPoint& pos, const wxSize& size ) :
                wxFrame( parent, id, title, pos, size )
{
    setupMenuBar();
}

void SideFrame::setupMenuBar()
{
    wxMenu* sessionMenu = new wxMenu();
    sessionMenu->Append( SideFrame::addVideoID,
                            _("Add video session...") );
    sessionMenu->Append( SideFrame::addAudioID,
                            _("Add audio session...") );
    sessionMenu->AppendSeparator();
    sessionMenu->Append( SideFrame::rotateID,
                            _("Rotate video sessions") );

    // add menubar to parent frame here - kind of clunky
    wxMenuBar* menubar = new wxMenuBar();
    menubar->Append( sessionMenu, _("Sessions") );

    SetMenuBar( menubar );

    Connect( addVideoID, wxEVT_COMMAND_MENU_SELECTED,
                  wxCommandEventHandler( SideFrame::addVideoSessionEvent ) );
    Connect( addAudioID, wxEVT_COMMAND_MENU_SELECTED,
                  wxCommandEventHandler( SideFrame::addAudioSessionEvent ) );
    Connect( rotateID, wxEVT_COMMAND_MENU_SELECTED,
              wxCommandEventHandler( SideFrame::rotateEvent ) );
}

SessionTreeControl* SideFrame::findSessionTree( wxWindow* w )
{
    SessionTreeControl* tree = dynamic_cast<SessionTreeControl*>( w );
    if ( tree != NULL )
        return tree;

    wxWindowList children = w->GetChildren();
    wxWindowList::iterator i = children.begin();
    for ( ; i != children.end(); ++i )
    {
        tree = findSessionTree( (*i ) );
        if ( tree != NULL )
            return tree;
    }

    return NULL;
}

// TODO make these use the same code and not be stupid - might necessitate
// making a custom dialog class or secondary radio button dialog choice
// note this has to be duplicated in SessionTreeControl
void SideFrame::addVideoSessionEvent( wxCommandEvent& evt )
{
    SessionTreeControl* tree = findSessionTree( this );
    if ( tree != NULL )
    {
        wxTextEntryDialog dialog( this,
                                 _("Enter session in format address/port"),
                                 _("Add New Video Session") );

        if ( dialog.ShowModal() == wxID_OK )
        {
            std::string address( dialog.GetValue().char_str() );
            tree->addSession( address, false, false );
        }
    }
}

void SideFrame::addAudioSessionEvent( wxCommandEvent& evt )
{
    SessionTreeControl* tree = findSessionTree( this );
    if ( tree != NULL )
    {
        wxTextEntryDialog dialog( this,
                                  _("Enter session in format address/port"),
                                  _("Add New Audio Session") );

        if ( dialog.ShowModal() == wxID_OK )
        {
            std::string address( dialog.GetValue().char_str() );
            tree->addSession( address, true, false );
        }
    }
}

void SideFrame::rotateEvent( wxCommandEvent& evt )
{
    SessionTreeControl* tree = findSessionTree( this );
    if ( tree != NULL )
        tree->rotateVideoSessions();
}
