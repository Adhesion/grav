/*
 * @file Group.h
 *
 * Represents a group of objects. The objects contained can be videos or other
 * groups (anything that is a RectangleBase). Objects in the group will be
 * drawn and moved together.
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

#ifndef GROUP_H_
#define GROUP_H_

#include "RectangleBase.h"
#include "LayoutManager.h"

#include <vector>

enum ArrangeStyle
{
    ASPECT,
    ONEROW,
    ONECOLUMN
};

class Group : public RectangleBase
{

public:
    Group( float _x, float _y );
    ~Group();

    virtual void draw();

    void add( RectangleBase* object );
    virtual void remove( RectangleBase* object, bool move = true );
    virtual std::vector<RectangleBase*>::iterator remove(
                    std::vector<RectangleBase*>::iterator i, bool move = true );
    void removeAll();

    RectangleBase* operator[]( int i );
    int numObjects();
    std::vector<RectangleBase*>::iterator getBeginIterator();
    std::vector<RectangleBase*>::iterator getEndIterator();

    bool isGroup();

    virtual void rearrange();
    ArrangeStyle getRearrange();
    void setRearrange( ArrangeStyle style );

    virtual bool updateName();

    void move( float _x, float _y );
    void setPos( float _x, float _y );
    void setScale( float xs, float ys );
    void setScale( float xs, float ys, bool resizeMembers );

    void show( bool s, bool instant = false );

protected:
    std::vector<RectangleBase*> objects;
    float buffer;
    LayoutManager layouts;

    ArrangeStyle rearrangeStyle;
    bool preserveChildAspect;

};

#endif /*GROUP_H_*/
