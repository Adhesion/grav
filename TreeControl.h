#ifndef TREECONTROL_H_
#define TREECONTROL_H_

/**
 * @file TreeControl.h
 * Definition of the TreeControl class. Defines behavior for the secondary
 * grav window that lists all sources, groups, etc. in a tree-like fashion.
 * @author Andrew Ford
 */

#include <wx/treectrl.h>
#include <string>

class gravManager;
class RectangleBase;

class TreeControl : public wxTreeCtrl
{

public:
    TreeControl( wxFrame* parent, gravManager* g );
    
    /*
     * Adds a new entry to represent a video or audio session. Sessions will
     * always be direct children of the root node.
     */
    void addSession( std::string address );
    
    void addObject( RectangleBase* obj );
    void removeObject( RectangleBase* obj );
    wxTreeItemId findObject( wxTreeItemId root, RectangleBase* obj );
    
    void updateObjectName( RectangleBase* obj );
    
private:
    wxTreeItemId rootID;
    gravManager* grav;
    
};

#endif /*TREECONTROL_H_*/
