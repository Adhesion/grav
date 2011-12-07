/*
 * @file Timers.cpp
 *
 * Implementation of various wx timers.
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

#include "Timers.h"
#include "SessionTreeControl.h"
#include "GLCanvas.h"
#include "gravUtil.h"

RenderTimer::RenderTimer( GLCanvas* c, int i ) :
    canvas( c ), interval( i )
{
    gettimeofday( &time, NULL );
    lastTimeMS = time.tv_usec;
}

void RenderTimer::Notify()
{
    canvas->draw();
    printTiming();
}

void RenderTimer::Start()
{
    gravUtil::logVerbose( "Timer::starting at interval %i\n", interval );
    wxTimer::Start( interval );
}

void RenderTimer::printTiming()
{
    time_t diff = getTiming();
    gravUtil::logVerbose( "%lu\n", (unsigned long)diff );

    resetTiming();
}

time_t RenderTimer::getTiming()
{
    gettimeofday( &time, NULL );
    time_t diff;

    if ( lastTimeMS > time.tv_usec )
        diff = (time.tv_usec+1000000) - lastTimeMS;
    else
        diff = time.tv_usec - lastTimeMS;

    return diff;
}

void RenderTimer::resetTiming()
{
    lastTimeMS = time.tv_usec;
}

RotateTimer::RotateTimer( SessionTreeControl* s ) :
    sessionTree( s )
{
    counterMax = 30000;
}

void RotateTimer::Notify()
{
    sessionTree->rotateVideoSessions( true );
    stopwatch.Start();
}

bool RotateTimer::Start( int milliseconds, bool oneShot )
{
    stopwatch.Start();
    if ( milliseconds != -1 )
        counterMax = milliseconds;
    return wxTimer::Start( milliseconds, oneShot );
}

float RotateTimer::getProgress()
{
    return (float)stopwatch.Time() / (float)counterMax;
}
