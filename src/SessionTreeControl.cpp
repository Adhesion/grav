/*
 * @file SessionTreeControl.cpp
 *
 * Implementation of the SessionTreeControl - see SessionTreeControl.h
 *
 * Created on: Sep 13, 2010
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

#include <wx/menu.h>
#include <wx/textdlg.h>

#include "SessionTreeControl.h"
#include "SessionManager.h"
#include "Frame.h"
#include "gravUtil.h"
#include "Timers.h"

int SessionTreeControl::addVideoID = wxNewId();
int SessionTreeControl::addAudioID = wxNewId();
int SessionTreeControl::toggleEnableID = wxNewId();
int SessionTreeControl::removeID = wxNewId();
int SessionTreeControl::rotateID = wxNewId();
int SessionTreeControl::rotateToID = wxNewId();
int SessionTreeControl::shiftID = wxNewId();
int SessionTreeControl::unrotateID = wxNewId();
int SessionTreeControl::toggleAutomaticRotateID = wxNewId();
int SessionTreeControl::setEncryptionID = wxNewId();
int SessionTreeControl::disableEncryptionID = wxNewId();

BEGIN_EVENT_TABLE(SessionTreeControl, wxTreeCtrl)
EVT_TREE_ITEM_RIGHT_CLICK(wxID_ANY, SessionTreeControl::itemRightClick)
END_EVENT_TABLE()

SessionTreeControl::SessionTreeControl() :
    wxTreeCtrl( NULL, wxID_ANY )
{
    timer = new RotateTimer( this );
}

SessionTreeControl::SessionTreeControl( wxWindow* parent ) :
    wxTreeCtrl( parent, wxID_ANY, parent->GetPosition(), parent->GetSize() )
{
    rootID = AddRoot( _("Sessions"), -1, -1, new wxTreeItemData() );
    videoNodeID = AppendItem( rootID, _("Video") );
    audioNodeID = AppendItem( rootID, _("Audio") );
    Expand( rootID );

    timer = new RotateTimer( this );
}

SessionTreeControl::~SessionTreeControl()
{
    delete timer;
    // note that we don't own the sessionmanager pointer
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
    SessionType type;

    // we don't account for audio + rotate since that's not supported
    if ( audio )
    {
        type = AUDIOSESSION;
        node = audioNodeID;
    }
    else if ( rotate )
    {
        type = AVAILABLEVIDEOSESSION;

        // make available video group if not there
        if ( !availableVideoNodeID.IsOk() )
        {
            availableVideoNodeID = AppendItem( rootID, _("Available Video") );
        }
        node = availableVideoNodeID;
    }
    else
    {
        type = VIDEOSESSION;
        node = videoNodeID;
    }

    added = sessionManager->addSession( address, type );

    if ( added )
    {
        current = AppendItem( node, wxString( address.c_str(), wxConvUTF8 ) );
        Expand( node );

        if ( rotate )
            SetItemBackgroundColour( current, *wxBLUE );
    }
    else
    {
        gravUtil::logError( "SessionTreeControl::addObject: "
                            "failed to initialize %s\n", address.c_str() );
        // TODO throw error dialog
    }
}

void SessionTreeControl::removeSession( std::string address )
{
    wxTreeItemId item = findSession( rootID, address );
    if ( !item.IsOk() )
    {
        gravUtil::logWarning( "SessionTreeControl::removeObject: "
                "item %s not found?\n", address.c_str() );
        return;
    }

    SessionType type;
    wxTreeItemId parent = GetItemParent( item );

    if ( parent == videoNodeID )
        type = VIDEOSESSION;
    else if ( parent == audioNodeID )
        type = AUDIOSESSION;
    else if ( parent == availableVideoNodeID )
        type = AVAILABLEVIDEOSESSION;
    else
    {
        gravUtil::logError( "SessionTreeControl::removeObject: "
                    "cannot remove %s - invalid tree\n", address.c_str() );
        return;
    }

    if ( sessionManager->removeSession( address, type ) )
        Delete( item );
    else
    {
        gravUtil::logError( "SessionTreeControl::removeObject: "
                        "removal of %s failed \n", address.c_str() );
        // TODO throw GUI error dialog
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

void SessionTreeControl::shiftSession( std::string address, bool audio )
{
    wxTreeItemId item = findSession( rootID, address );
    if ( item.IsOk() )
    {
        wxTreeItemId parent = GetItemParent( item );
        wxTreeItemId newParent;
        SessionType type;

        if ( parent == videoNodeID )
        {
            type = VIDEOSESSION;

            // make available video group if not there
            if ( !availableVideoNodeID.IsOk() )
            {
                availableVideoNodeID = AppendItem( rootID,
                                                    _("Available Video") );
            }

            newParent = availableVideoNodeID;
        }
        else if ( parent == availableVideoNodeID )
        {
            type = AVAILABLEVIDEOSESSION;
            newParent = videoNodeID;
        }
        else
        {
            gravUtil::logError( "SessionTreeControl::shiftSession: "
                                "cannot shift %s, invalid tree position\n",
                                address.c_str() );
            return;
        }

        // disconnect from availables and stop rotating (if auto), since moving
        // sessions around implies we want to make sure too many sessions aren't
        // running at once
        if ( timer->IsRunning() )
        {
            toggleAutomaticRotate();
        }
        unrotateVideoSessions();

        if ( !sessionManager->shiftSession( address, type ) )
        {
            gravUtil::logError( "SessionTreeControl::shiftSession: "
                                "shift of %s failed \n", address.c_str() );
            return;
        }

        Delete( item );

        wxTreeItemId newNode = AppendItem( newParent,
                wxString( address.c_str(), wxConvUTF8 ) );
        Expand( newParent );

        if ( newParent == availableVideoNodeID )
            SetItemBackgroundColour( newNode, *wxBLUE );
    }
    else
    {
        gravUtil::logWarning( "SessionTreeControl::shiftSession: "
                                "item %s not found?\n", address.c_str() );
    }
}

void SessionTreeControl::rotateVideoSessions()
{
    sessionManager->rotate( false );

    wxTreeItemId current = findSession( availableVideoNodeID,
            sessionManager->getCurrentRotateSessionAddress() );
    wxTreeItemId last = findSession( availableVideoNodeID,
            sessionManager->getLastRotateSessionAddress() );

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

void SessionTreeControl::rotateToVideoSession( std::string addr )
{
    sessionManager->rotateTo( addr, false );

    wxTreeItemId current = findSession( availableVideoNodeID,
            sessionManager->getCurrentRotateSessionAddress() );
    wxTreeItemId last = findSession( availableVideoNodeID,
            sessionManager->getLastRotateSessionAddress() );

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

void SessionTreeControl::unrotateVideoSessions()
{
    wxTreeItemId current = findSession( availableVideoNodeID,
            sessionManager->getCurrentRotateSessionAddress() );
    if ( current.IsOk() )
    {
        SetItemBackgroundColour( current, *wxBLUE );
        SetItemTextColour( current, *wxBLACK );
    }

    sessionManager->unrotate( false );
}

void SessionTreeControl::toggleAutomaticRotate()
{
    if ( timer->IsRunning() )
    {
        timer->Stop();
    }
    else
    {
        timer->Start( -1 );
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
    // checking parent prevents remove option from showing up on top-level
    // group
    if ( parent.IsOk() &&
            ( parent == videoNodeID || parent == audioNodeID ||
              parent == availableVideoNodeID ) )
    {
        // only add enable/disable if it's a real session, not a available one
        // also add encryption set
        if ( parent == videoNodeID || parent == audioNodeID )
        {
            wxString toggleLabel =
                    sessionManager->isSessionProcessEnabled( text ) ?
                            _("Disable") : _("Enable");
            rightClickMenu.Append( toggleEnableID, toggleLabel );
            Connect( toggleEnableID, wxEVT_COMMAND_MENU_SELECTED,
                        wxCommandEventHandler(
                                SessionTreeControl::toggleEnableSessionEvent) );

            // only add shift option for video for now
            if ( parent == videoNodeID )
            {
                rightClickMenu.Append( shiftID,
                    _("Shift session to available video list") );
                Connect( shiftID, wxEVT_COMMAND_MENU_SELECTED,
                    wxCommandEventHandler( SessionTreeControl::shiftEvent ) );
            }

            rightClickMenu.Append( setEncryptionID,
                                    _("Set encryption key...") );
            Connect( setEncryptionID, wxEVT_COMMAND_MENU_SELECTED,
                        wxCommandEventHandler(
                                SessionTreeControl::setEncryptionEvent ) );

            // add disable encryption option for sessions with encryption
            if ( sessionManager->isEncryptionEnabled( text ) )
            {
                rightClickMenu.Append( disableEncryptionID,
                                        _("Disable encryption") );
                Connect( disableEncryptionID, wxEVT_COMMAND_MENU_SELECTED,
                            wxCommandEventHandler(
                                SessionTreeControl::disableEncryptionEvent ) );
            }
        }
        // for available video
        else if ( parent == availableVideoNodeID )
        {
            // for now, have "rotate to this" for all but selected
            if ( text.compare(
                    sessionManager->getCurrentRotateSessionAddress() ) != 0 )
            {
                rightClickMenu.Append( rotateToID,
                                        _("Rotate to this session") );
                Connect( rotateToID, wxEVT_COMMAND_MENU_SELECTED,
                            wxCommandEventHandler(
                                  SessionTreeControl::rotateToEvent ) );
            }

            rightClickMenu.Append( shiftID, _("Shift session to main list") );
            Connect( shiftID, wxEVT_COMMAND_MENU_SELECTED,
                    wxCommandEventHandler( SessionTreeControl::shiftEvent ) );
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

    // right click on available video group
    if ( item == availableVideoNodeID )
    {
        rightClickMenu.Append( rotateID, _("Rotate video sessions") );
        Connect( rotateID, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler( SessionTreeControl::rotateEvent ) );

        wxMenuItem* automaticItem = rightClickMenu.AppendCheckItem(
            toggleAutomaticRotateID, _("Automatically rotate sessions") );
        Connect( toggleAutomaticRotateID, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler(
                SessionTreeControl::toggleAutomaticRotateEvent ) );
        automaticItem->Check( timer->IsRunning() );

        wxMenuItem* unrotateItem = rightClickMenu.Append( unrotateID,
            _("Disconnect from available video session") );
        Connect( unrotateID, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler( SessionTreeControl::unrotateEvent ) );
        unrotateItem->Enable(
            sessionManager->getCurrentRotateSessionAddress().compare( "" ) !=
                    0 );
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
    if ( sessionManager->setSessionProcessEnable( selectedAddress,
            !sessionManager->isSessionProcessEnabled( selectedAddress ) ) )
    {
        wxColour col =
                sessionManager->isSessionProcessEnabled( selectedAddress ) ?
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
    rotateVideoSessions();
}

void SessionTreeControl::rotateToEvent( wxCommandEvent& evt )
{
    std::string selectedAddress = std::string(
                                     GetItemText( GetSelection() ).char_str() );
    rotateToVideoSession( selectedAddress );
}

void SessionTreeControl::shiftEvent( wxCommandEvent& evt )
{
    std::string selectedAddress = std::string(
                                     GetItemText( GetSelection() ).char_str() );
    shiftSession( selectedAddress, false );
}

void SessionTreeControl::unrotateEvent( wxCommandEvent& evt )
{
    unrotateVideoSessions();
}

void SessionTreeControl::toggleAutomaticRotateEvent( wxCommandEvent& evt )
{
    toggleAutomaticRotate();
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

void SessionTreeControl::startTimer( int ms )
{
    timer->Start( ms );
}

void SessionTreeControl::stopTimer()
{
    timer->Stop();
}
