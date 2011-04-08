/*
 * @file TreeControl.h
 *
 * Definition of the TreeControl class. Defines behavior for the secondary
 * grav window that lists all sources, groups, etc. in a tree-like fashion.
 *
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

#ifndef TREECONTROL_H_
#define TREECONTROL_H_

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
