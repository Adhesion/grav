/*
 * SideFrame.cpp
 *
 * Implementation of the side GUI window.
 *
 * Created on: Oct 12, 2010
 * @author Andrew Ford
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
