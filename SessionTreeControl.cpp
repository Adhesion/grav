/**
 * @file SessionTreeControl.cpp
 *
 * Implementation of the SessionTreeControl - see SessionTreeControl.h
 *
 * Created on: Sep 13, 2010
 * @author Andrew Ford
 */

#include "SessionTreeControl.h"

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

void SessionTreeControl::addSession( std::string address, bool audio )
{
    if ( !audio )
    {
        AppendItem( videoNodeID, wxString( address.c_str(), wxConvUTF8 ) );
        Expand( videoNodeID );
    }
    else
    {
        AppendItem( audioNodeID, wxString( address.c_str(), wxConvUTF8 ) );
        Expand( audioNodeID );
    }
}
