/**
 * @file SessionTreeControl.cpp
 *
 * Implementation of the SessionTreeControl - see SessionTreeControl.h
 *
 * Created on: Sep 13, 2010
 * @author Andrew Ford
 */

#include <wx/menu.h>
#include <wx/textdlg.h>

#include "SessionTreeControl.h"
#include "SessionManager.h"
#include "Frame.h"

BEGIN_EVENT_TABLE(SessionTreeControl, wxTreeCtrl)
EVT_TREE_ITEM_RIGHT_CLICK(wxID_ANY, SessionTreeControl::itemRightClick)
END_EVENT_TABLE()

int SessionTreeControl::addVideoID = wxNewId();
int SessionTreeControl::addAudioID = wxNewId();
int SessionTreeControl::toggleEnableID = wxNewId();
int SessionTreeControl::removeID = wxNewId();
int SessionTreeControl::rotateID = wxNewId();

SessionTreeControl::SessionTreeControl() :
    wxTreeCtrl( NULL, wxID_ANY )
{ }

SessionTreeControl::SessionTreeControl( wxWindow* parent ) :
    wxTreeCtrl( parent, wxID_ANY, parent->GetPosition(), parent->GetSize() )
{
    rootID = AddRoot( _("Sessions"), -1, -1, new wxTreeItemData() );
    videoNodeID = AppendItem( rootID, _("Video") );
    audioNodeID = AppendItem( rootID, _("Audio") );
    Expand( rootID );
}

void SessionTreeControl::setSessionManager( SessionManager* s )
{
    sessionManager = s;
}

void SessionTreeControl::addSession( std::string address, bool audio,
                                        bool rotate )
{
    bool added = false;
    wxTreeItemId node;
    wxTreeItemId current;
    if ( rotate )
    {
        // rotate only for video for now
        node = videoNodeID;
        sessionManager->addRotatedSession( address, false );
        added = true;
    }
    else
    {
        node = audio ? audioNodeID : videoNodeID;
        added = sessionManager->initSession( address, audio );
    }

    if ( added )
    {
        current = AppendItem( node, wxString( address.c_str(),
                                            wxConvUTF8 ) );
        Expand( node );
    }
    else
    {
        // TODO throw error dialog
    }

    if ( rotate )
        SetItemBackgroundColour( current, *wxBLUE );
}

void SessionTreeControl::removeSession( std::string address )
{
    wxTreeItemId item = findSession( rootID, address );
    if ( !item.IsOk() )
    {
        printf( "SessionTreeControl::removeObject: ERROR: item %s not found?\n",
                    address.c_str() );
        return;
    }

    if ( sessionManager->removeSession( address ) )
    {
        Delete( item );
    }
    else
    {
        // TODO throw error dialog
    }
}

wxTreeItemId SessionTreeControl::findSession( wxTreeItemId root,
                                                std::string address )
{
    wxTreeItemIdValue temp; // unused var, needed in getchild
    wxTreeItemId targetItem;
    wxTreeItemId current = GetFirstChild( root, temp );

    while ( current.IsOk() )
    {
        wxString text = GetItemText( current );
        std::string target = std::string( text.char_str() );
        if ( target.compare( address ) == 0 )
            return current;

        if ( ItemHasChildren( current ) )
        {
            targetItem = findSession( current, address );
            if ( targetItem.IsOk() )
                return targetItem;
        }
        current = GetNextChild( root, temp );
    }

    wxTreeItemId none;
    return none; // return default value if not found
}

void SessionTreeControl::rotateVideoSessions()
{
    sessionManager->rotate( false );

    wxTreeItemId current = findSession( videoNodeID,
                               sessionManager->getCurrentRotateSession() );
    wxTreeItemId last =    findSession( videoNodeID,
                               sessionManager->getLastRotateSession() );

    if ( current.IsOk() )
        SetItemBackgroundColour( current, *wxRED );
    if ( last.IsOk() )
        SetItemBackgroundColour( last, *wxBLUE );
}

void SessionTreeControl::itemRightClick( wxTreeEvent& evt )
{
    wxMenu rightClickMenu;

    std::string text = std::string( GetItemText( evt.GetItem() ).char_str() );
    // when right clicking on a session
    if ( text.compare( "Video" ) != 0 && text.compare( "Audio" ) != 0 &&
            text.compare( "Sessions" ) != 0 )
    {
        wxString toggleLabel = sessionManager->isSessionEnabled( text ) ?
                _("Disable") : _("Enable");
        rightClickMenu.Append( toggleEnableID, toggleLabel );
        rightClickMenu.Append( removeID, _("Remove") );
        // TODO static evt_menu binding (at top) doesn't seem to work for
        // these, so doing runtime connect
        Connect( toggleEnableID, wxEVT_COMMAND_MENU_SELECTED,
                    wxCommandEventHandler(
                            SessionTreeControl::toggleEnableSessionEvent) );
        Connect( removeID, wxEVT_COMMAND_MENU_SELECTED,
                    wxCommandEventHandler(
                            SessionTreeControl::removeSessionEvent ) );
    }
    // right clicked on video group or main group
    if ( text.compare( "Video" ) == 0 || text.compare( "Sessions" ) == 0 )
    {
        rightClickMenu.Append( addVideoID, _("Add video session...") );
        Connect( addVideoID, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler( SessionTreeControl::addVideoSessionEvent ) );
        rightClickMenu.Append( rotateID, _("Rotate video sessions") );
        Connect( rotateID, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler( SessionTreeControl::rotateEvent ) );
    }
    // right clicked on audio group or main group
    if ( text.compare( "Audio" ) == 0 || text.compare( "Sessions" ) == 0 )
    {
        rightClickMenu.Append( addAudioID, _("Add audio session...") );
        Connect( addAudioID, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler( SessionTreeControl::addAudioSessionEvent ) );
    }

    PopupMenu( &rightClickMenu, evt.GetPoint() );
}

// TODO make these use the same code and not be stupid - might necessitate
// making a custom dialog class or secondary radio button dialog choice
// note this is duplicated in SideFrame
void SessionTreeControl::addVideoSessionEvent( wxCommandEvent& evt )
{
    wxTextEntryDialog dialog( this, _("Enter session in format address/port"),
                                _("Add New Video Session") );

    if ( dialog.ShowModal() == wxID_OK )
    {
        std::string address( dialog.GetValue().char_str() );
        addSession( address, false, false );
    }
}

void SessionTreeControl::addAudioSessionEvent( wxCommandEvent& evt )
{
    wxTextEntryDialog dialog( this, _("Enter session in format address/port"),
                                _("Add New Audio Session") );

    if ( dialog.ShowModal() == wxID_OK )
    {
        std::string address( dialog.GetValue().char_str() );
        addSession( address, true, false );
    }
}

void SessionTreeControl::toggleEnableSessionEvent( wxCommandEvent& evt )
{
    std::string selectedAddress = std::string(
                                     GetItemText( GetSelection() ).char_str() );
    if ( sessionManager->setSessionEnable( selectedAddress,
            !sessionManager->isSessionEnabled( selectedAddress ) ) )
    {
        wxColour col = sessionManager->isSessionEnabled( selectedAddress ) ?
                        *wxWHITE : *wxLIGHT_GREY;
        SetItemBackgroundColour( GetSelection(), col );
    }
}

void SessionTreeControl::removeSessionEvent( wxCommandEvent& evt )
{
    std::string selectedAddress = std::string(
                                     GetItemText( GetSelection() ).char_str() );
    removeSession( selectedAddress );
}

void SessionTreeControl::rotateEvent( wxCommandEvent& evt )
{
    printf( "rotate event: have %i sessions\n",
            sessionManager->getVideoSessionCount() );
    rotateVideoSessions();
}
