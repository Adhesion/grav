/**
 * @file TreeNode.h
 * Implements a node for the wxTreeControl.
 * @author Andrew Ford
 */

#include "TreeNode.h"

TreeNode::TreeNode( RectangleBase* obj, bool session )
    : isSession( session ), object( obj )
{ }

RectangleBase* TreeNode::getObject()
{
    return object;
}
