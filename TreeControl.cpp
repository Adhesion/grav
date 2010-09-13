/**
 * @file TreeControl.cpp
 * Implementation of the TreeControl class. Defines methods for searching &
 * adding items in the tree.
 * @author Andrew Ford
 */

#include "RectangleBase.h"
#include "TreeControl.h"
#include "gravManager.h"
#include "TreeNode.h"
#include "Runway.h"

#include <wx/wx.h>

IMPLEMENT_DYNAMIC_CLASS( TreeControl, wxTreeCtrl )

TreeControl::TreeControl() :
    wxTreeCtrl( NULL, wxID_ANY )
{ }

TreeControl::TreeControl( wxWindow* parent ) :
    wxTreeCtrl( parent, wxID_ANY, parent->GetPosition(), parent->GetSize() )
{
    rootID = AddRoot( _("Groups"), -1, -1, new wxTreeItemData() );
    //SetWindowStyle( GetWindowStyle() | wxTR_HIDE_ROOT );
}

TreeControl::~TreeControl()
{
    sourceManager->setTree( NULL );
}

void TreeControl::addSession( std::string address )
{
    AppendItem( rootID, wxString( address.c_str(), wxConvUTF8 ),
                -1, -1, new TreeNode( NULL, true ) );
}

void TreeControl::addObject( RectangleBase* obj )
{
    wxTreeItemId parentID;
    
    // if it's not grouped, or it's in the runway, we can add it to root
    if ( !obj->isGrouped() || ( obj->isGrouped() &&
                            dynamic_cast<Runway*>( obj->getGroup() ) != NULL ) )
    {
        parentID = rootID;
    }
    // otherwise, find where it should be
    else
    {
        RectangleBase* parent = (RectangleBase*)obj->getGroup();
        parentID = findObject( rootID, parent );
    }
    
    if ( parentID.IsOk() )
    {
        wxTreeItemId newItem = AppendItem( parentID,
                        wxString( obj->getName().c_str(), wxConvUTF8 ),
                        -1, -1, new TreeNode( obj, false ) );
        if ( obj->getName() == "" )
            SetItemText( newItem, _( "(waiting for name...)" ) );
        SortChildren( parentID );
        
        // if we're going from 1 to 2 objects (1 to 2 objects in the tree
        // means 0 to 1 sources since the root node counts as an object)
        // expand the root level automatically so it'll be expanded by
        // default, but also if the number goes back to 0 and up again
        if ( GetCount() == 2 )
            Expand( rootID );
    }
    else
    {
        printf( "TreeControl::addObject: parent NOT found\n" );
    }
}

void TreeControl::removeObject( RectangleBase* obj )
{
    wxTreeItemId item = findObject( rootID, obj );
    if ( !item.IsOk() )
    {
        printf( "TreeControl::removeObject: ERROR: item %s not found?\n",
                    obj->getName().c_str() );
        return;
    }
    
    // if we're removing a group, take all of its children and add them to
    // root
    if ( obj->isGroup() )
    {
        wxTreeItemId parent = rootID;
        wxTreeItemIdValue temp;
        wxTreeItemId current = GetFirstChild( item, temp );
        
        while ( current.IsOk() )
        {
            TreeNode* data = dynamic_cast<TreeNode*>( GetItemData( current ) );
            if ( data != NULL )
            {
                RectangleBase* obj = data->getObject();
                addObject( obj );
            }
            current = GetNextChild( item, temp );
        }
        
        DeleteChildren( item );
    }
    
    Delete( item );
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
    SortChildren( GetItemParent( item ) );
}

int TreeControl::OnCompareItems( const wxTreeItemId& item1,
                                    const wxTreeItemId& item2 )
{
    TreeNode* node1 = dynamic_cast<TreeNode*>(GetItemData( item1 ));
    TreeNode* node2 = dynamic_cast<TreeNode*>(GetItemData( item2 ));
    if ( node1 == NULL || node2 == NULL )
        return 0;
    
    RectangleBase* obj1 = node1->getObject();
    RectangleBase* obj2 = node2->getObject();
    if ( obj1 == NULL || obj2 == NULL ) return 0;
    bool group1 = obj1->isGroup();
    bool group2 = obj2->isGroup();
    
    // sort groups higher than non-groups, and if they're the same type,
    // just sort alphabetically by name
    if ( group1 && !group2 )
        return -1;
    else if ( !group1 && group2 )
        return 1;
    else
        return obj1->getName().compare( obj2->getName() );
}

void TreeControl::setSourceManager( gravManager* g )
{
    sourceManager = g;
}
