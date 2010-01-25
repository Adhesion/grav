#ifndef TREENODE_H_
#define TREENODE_H_

/**
 * @file TreeNode.h
 * Defines a node for the wxTreeCtrl tree, either holding a RectangleBase
 * object or representing a multicast session.
 * @author Andrew Ford
 */

#include <wx/treectrl.h>

class RectangleBase;

class TreeNode : public wxTreeItemData
{
    
public:
    TreeNode( RectangleBase* obj, bool session );
    RectangleBase* getObject();
    
private:
    bool isSession;
    RectangleBase* object;
    
};

#endif /*TREENODE_H_*/
