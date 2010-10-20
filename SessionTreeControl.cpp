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
int SessionTreeControl::setEncryptionID = wxNewId();
int SessionTreeControl::disableEncryptionID = wxNewId();

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
    printf( "tree control adding session, rotate is %i\n", rotate );
    bool added = false;
    wxTreeItemId node;
    wxTreeItemId current;
    if ( rotate )
    {
        // make rotated video group if not there
        if ( !rotatedVideoNodeID.IsOk() )
        {
            rotatedVideoNodeID = AppendItem( videoNodeID, _("Rotated Video") );
            Expand( videoNodeID );
        }
        node = rotatedVideoNodeID;
        // note rotate is only for video for now
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

    // figure out whether it's a rotated session or not here
    if ( GetItemParent( item ) == videoNodeID )
    {
        if ( sessionManager->removeSession( address ) )
            Delete( item );
        else
        {
            // TODO throw error dialog, maybe make consistent with part below
        }
    }
    else if ( GetItemParent( item ) == rotatedVideoNodeID )
    {
        sessionManager->removeRotatedSession( address, false );
        Delete( item );
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

    if ( last.IsOk() )
    {
        SetItemBackgroundColour( last, *wxBLUE );
        SetItemTextColour( last, *wxBLACK );
    }
    if ( current.IsOk() )
    {
        SetItemBackgroundColour( current, *wxWHITE );
        SetItemTextColour( current, *wxBLUE );
    }
}

bool SessionTreeControl::setEncryptionKey( std::string addr, std::string key )
{
    return sessionManager->setEncryptionKey( addr, key );
}

bool SessionTreeControl::disableEncryption( std::string addr )
{
    return sessionManager->disableEncryption( addr );
}

void SessionTreeControl::itemRightClick( wxTreeEvent& evt )
{
    wxMenu rightClickMenu;

    std::string text = std::string( GetItemText( evt.GetItem() ).char_str() );
    wxTreeItemId item = evt.GetItem();
    wxTreeItemId parent = GetItemParent( item );

    // when right clicking on a session
    if ( ( parent == videoNodeID || parent == audioNodeID ||
            parent == rotatedVideoNodeID ) && item != rotatedVideoNodeID )
    {
        // only add enable/disable if it's a real session, not a rotated one
        // also add encryption set
        if ( parent == videoNodeID || parent == audioNodeID )
        {
            wxString toggleLabel = sessionManager->isSessionEnabled( text ) ?
                            _("Disable") : _("Enable");
            rightClickMenu.Append( toggleEnableID, toggleLabel );
            Connect( toggleEnableID, wxEVT_COMMAND_MENU_SELECTED,
                        wxCommandEventHandler(
                                SessionTreeControl::toggleEnableSessionEvent) );

            rightClickMenu.Append( setEncryptionID,
                                    _("Set encryption key...") );
            Connect( setEncryptionID, wxEVT_COMMAND_MENU_SELECTED,
                        wxCommandEventHandler(
                                SessionTreeControl::setEncryptionEvent) );

            // add disable encryption option for sessions with encryption
            if ( sessionManager->isEncryptionEnabled( text ) )
            {
                rightClickMenu.Append( disableEncryptionID,
                                        _("Disable encryption") );
                Connect( disableEncryptionID, wxEVT_COMMAND_MENU_SELECTED,
                            wxCommandEventHandler(
                                  SessionTreeControl::disableEncryptionEvent) );
            }
        }

        rightClickMenu.Append( removeID, _("Remove") );
        // TODO static evt_menu binding (at top) doesn't seem to work for
        // these, so doing runtime connect
        Connect( removeID, wxEVT_COMMAND_MENU_SELECTED,
                    wxCommandEventHandler(
                            SessionTreeControl::removeSessionEvent ) );
    }

    // right clicked on video group or main group
    if ( item == videoNodeID || item == rootID )
    {
        rightClickMenu.Append( addVideoID, _("Add video session...") );
        Connect( addVideoID, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler( SessionTreeControl::addVideoSessionEvent ) );
    }

    // right click on rotated video group
    if ( item == rotatedVideoNodeID )
    {
        rightClickMenu.Append( rotateID, _("Rotate video sessions") );
        Connect( rotateID, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler( SessionTreeControl::rotateEvent ) );
    }

    // right clicked on audio group or main group
    if ( item == audioNodeID || item == rootID )
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

void SessionTreeControl::setEncryptionEvent( wxCommandEvent& evt )
{
    std::string selectedAddress = std::string(
                                    GetItemText( GetSelection() ).char_str() );
    std::string desc = "Set encryption key for " + selectedAddress;
    wxTextEntryDialog dialog( this, _("Enter encryption key"),
                                wxString( desc.c_str(), wxConvUTF8 ) );

    if ( dialog.ShowModal() == wxID_OK )
    {
        std::string key( dialog.GetValue().char_str() );
        setEncryptionKey( selectedAddress, key );
    }
}

void SessionTreeControl::disableEncryptionEvent( wxCommandEvent& evt )
{
    std::string selectedAddress = std::string(
                                    GetItemText( GetSelection() ).char_str() );
    disableEncryption( selectedAddress );
}
