/*
 * @file SessionGroupButton.h
 *
 * Definition of a class to act as an interactive GUI element attached to a
 * SessionGroup.
 *
 * Created on: Nov 07, 2011
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

#ifndef SESSIONGROUPBUTTON_H_
#define SESSIONGROUPBUTTON_H_

#include "RectangleBase.h"

class SessionGroup;

class SessionGroupButton : public RectangleBase
{

public:
    SessionGroupButton( float _x, float _y );

    void draw();

    void doubleClickAction();

    void setControlledGroup( SessionGroup* g );
    SessionGroup* getControlledGroup();

    void setPlaying( bool p );

private:
    // note this is separate from the regular RectBase group/parent
    SessionGroup* controlledGroup;

    bool playing;

};

#endif /* SESSIONGROUPBUTTON_H_ */
