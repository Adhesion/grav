/*
 * @file Timers.h
 *
 * Definition of various wx timers.
 *
 * Created on: Oct 15, 2010
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

#ifndef TIMERS_H_
#define TIMERS_H_

#include <wx/timer.h>

class SessionTreeControl;
class GLCanvas;

class RenderTimer : public wxTimer
{

public:
    RenderTimer( GLCanvas* c, int i );
    void Notify();
    void Start();

    // print number of microseconds since last call
    void printTiming();
    time_t getTiming();
    void resetTiming();

private:
    GLCanvas* canvas;

    // interval between timer firing, in milliseconds
    int interval;

    struct timeval time;
    time_t lastTimeMS;

};

class RotateTimer : public wxTimer
{

public:
    RotateTimer( SessionTreeControl* s );
    void Notify();

    // override start so we notify immediately when starting, rather than
    // waiting for the first interval to pass
    bool Start( int milliseconds = -1, bool oneShot = false );

private:
    SessionTreeControl* sessionTree;

};

#endif /* TIMERS_H_ */
