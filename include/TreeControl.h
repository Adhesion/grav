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
    DECLARE_DYNAMIC_CLASS( TreeControl )

public:
    TreeControl();
    TreeControl( wxWindow* parent );

    ~TreeControl();

    void addObject( RectangleBase* obj );
    void removeObject( RectangleBase* obj );
    wxTreeItemId findObject( wxTreeItemId root, RectangleBase* obj );
    
    void updateObjectName( RectangleBase* obj );
    
    /*
     * Overrides the wxTreeCtrl's compare function to implement a custom
     * sort.
     */
    int OnCompareItems( const wxTreeItemId& item1,
                            const wxTreeItemId& item2 );

    void setSourceManager( gravManager* g );
    
private:
    wxTreeItemId rootID;
    gravManager* sourceManager;
    
};

#endif /*TREECONTROL_H_*/
