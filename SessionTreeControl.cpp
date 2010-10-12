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

    wxMenu* sessionMenu = new wxMenu();
    sessionMenu->Append( addVideoID, _("Add video session") );
    sessionMenu->Append( addAudioID, _("Add audio session") );

    // add menubar to parent frame here - kind of clunky
    wxMenuBar* menubar = new wxMenuBar;
    menubar->Append( sessionMenu, _("Sessions") );
    wxWindow* tempParent = this;
    while ( tempParent && dynamic_cast<Frame*>( tempParent ) == NULL )
        tempParent = tempParent->GetParent();
    Frame* topFrame = dynamic_cast<Frame*>( tempParent );
    if ( topFrame != NULL )
    {
        Frame* topFrame = dynamic_cast<Frame*>( tempParent );
        topFrame->SetMenuBar( menubar );
    }
}

void SessionTreeControl::setSessionManager( SessionManager* s )
{
    sessionManager = s;
}

void SessionTreeControl::addSession( std::string address, bool audio )
{
    if ( !audio )
    {
        if ( sessionManager->initSession( address, false ) )
        {
            AppendItem( videoNodeID, wxString( address.c_str(), wxConvUTF8 ) );
            Expand( videoNodeID );
        }
        else
        {
            // TODO throw error dialog
        }
    }
    else
    {
        if ( sessionManager->initSession( address, true ) )
        {
            AppendItem( audioNodeID, wxString( address.c_str(), wxConvUTF8 ) );
            Expand( audioNodeID );
        }
        else
        {
            // TODO throw error dialog
        }
    }
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

void SessionTreeControl::itemRightClick( wxTreeEvent& evt )
{
    wxMenu rightClickMenu;

    std::string text = std::string( GetItemText( evt.GetItem() ).char_str() );
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
    if ( text.compare( "Video" ) == 0 || text.compare( "Sessions" ) == 0 )
    {
        rightClickMenu.Append( addVideoID, _("Add video session") );
        Connect( addVideoID, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler( SessionTreeControl::addVideoSessionEvent ) );
    }
    if ( text.compare( "Audio" ) == 0 || text.compare( "Sessions" ) == 0 )
    {
        rightClickMenu.Append( addAudioID, _("Add audio session") );
        Connect( addAudioID, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler( SessionTreeControl::addAudioSessionEvent ) );
    }

    PopupMenu( &rightClickMenu, evt.GetPoint() );
}

// TODO make these use the same code and not be stupid - might necessitate
// making a custom dialog class or secondary radio button dialog choice
void SessionTreeControl::addVideoSessionEvent( wxCommandEvent& evt )
{
    wxTextEntryDialog dialog( this, _("Enter session in format address/port"),
                                _("Add New Video Session") );

    if ( dialog.ShowModal() == wxID_OK )
    {
        std::string address( dialog.GetValue().char_str() );
        addSession( address, false );
    }
}

void SessionTreeControl::addAudioSessionEvent( wxCommandEvent& evt )
{
    wxTextEntryDialog dialog( this, _("Enter session in format address/port"),
                                _("Add New Audio Session") );

    if ( dialog.ShowModal() == wxID_OK )
    {
        std::string address( dialog.GetValue().char_str() );
        addSession( address, true );
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
