/*
 * @file TreeNode.h
 *
 * Defines a node for the wxTreeCtrl tree, either holding a RectangleBase
 * object or representing a multicast session.
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

#ifndef TREENODE_H_
#define TREENODE_H_

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
