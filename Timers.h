/**
 * Timers.h
 *
 * Definition of various wx timers.
 *
 * Created on: Oct 15, 2010
 * @author Andrew Ford
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
