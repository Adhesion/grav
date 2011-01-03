/**
 * Timers.cpp
 *
 * Implementation of various wx timers.
 *
 * Created on: Oct 15, 2010
 * @author Andrew Ford
 */

#include "Timers.h"
#include "SessionTreeControl.h"
#include "GLCanvas.h"

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
    printf( "Timer::starting at interval %i\n", interval);
    wxTimer::Start( interval );
}

void RenderTimer::printTiming()
{
    time_t diff = getTiming();
    printf( "%lu\n", (unsigned long)diff );

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

}

void RotateTimer::Notify()
{
    sessionTree->rotateVideoSessions();
}

bool RotateTimer::Start( int milliseconds, bool oneShot )
{
    Notify();
    return wxTimer::Start( milliseconds, oneShot );
}
