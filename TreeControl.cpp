/**
 * @file TreeControl.cpp
 * Implementation of the TreeControl class. Defines methods for searching &
 * adding items in the tree.
 * @author Andrew Ford
 */

#include "TreeControl.h"
#include "glutVideo.h"
#include "TreeNode.h"
#include "RectangleBase.h"

#include <wx/wx.h>

TreeControl::TreeControl( wxFrame* parent, gravManager* g ) :
    wxTreeCtrl( parent, wxID_ANY ), grav( g )
{
    rootID = AddRoot(_("grav::Groups"), -1, -1, new wxTreeItemData() );
    SetSize( parent->GetSize() );
    //SetWindowStyle( GetWindowStyle() | wxTR_HIDE_ROOT );
}

void TreeControl::addSession( std::string address )
{
    AppendItem( rootID, wxString( address.c_str(), wxConvUTF8 ),
                -1, -1, new TreeNode( NULL, true ) );
}

void TreeControl::addObject( RectangleBase* obj )
{
    wxTreeItemId parentID;
    
    // if it's not grouped we can add it to root
    if ( !obj->isGrouped() )
    {
        parentID = rootID;
    }
    // otherwise, find where it should be
    else
    {
        RectangleBase* parent = (RectangleBase*)obj->getGroup();
        wxTreeItemId parentID = findObject( rootID, parent );
    }
    
    if ( parentID.IsOk() )
    {
        wxTreeItemId newItem = AppendItem( parentID,
                        wxString( obj->getName().c_str(), wxConvUTF8 ),
                        -1, -1, new TreeNode( obj, false ) );
        if ( obj->getName() == "" )
            SetItemText( newItem, _( "(waiting for name...)" ) );
    }
}

void TreeControl::removeObject( RectangleBase* obj )
{
    Delete( findObject( rootID, obj ) );
}

wxTreeItemId TreeControl::findObject( wxTreeItemId root, RectangleBase* obj )
{
    wxTreeItemIdValue temp; // unused var, needed in getchild
    wxTreeItemId targetItem;
    wxTreeItemId current = GetFirstChild( root, temp );
    
    while ( current.IsOk() )
    {
        TreeNode* data = dynamic_cast<TreeNode*>( GetItemData( current ) );
        if ( data != NULL )
        {
            RectangleBase* target = data->getObject();
            if ( target == obj ) return current;
        }
        if ( ItemHasChildren( current ) )
        {
            targetItem = findObject( current, obj );
            if ( targetItem.IsOk() )
                return targetItem;
        }
        current = GetNextChild( root, temp );
    }
    
    wxTreeItemId none;
    return none; // return default value if not found
}

void TreeControl::updateObjectName( RectangleBase* obj )
{
    wxTreeItemId item = findObject( rootID, obj );
    SetItemText( item, wxString( obj->getName().c_str(), wxConvUTF8 ) );
}
