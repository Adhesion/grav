#ifndef GLCANVAS_H_
#define GLCANVAS_H_

/**
 * @file GLCanvas.h
 * Declaration of the GL canvas class, which defines the main rendered area
 * and draws it.
 * @author Andrew Ford
 */

#include "GLUtil.h"

#include <wx/glcanvas.h>

#include <sys/time.h>

class gravManager;
class RenderTimer;

class GLCanvas : public wxGLCanvas
{

public:
    GLCanvas( wxWindow* parent, gravManager* g, int* attributes, wxSize size );
    ~GLCanvas();

    void handlePaintEvent( wxPaintEvent& evt );
    void draw();
    void resize( wxSizeEvent& evt );
    void GLreshape( int w, int h );

    void stopTimer();
    void setTimer( RenderTimer* t );

    long getDrawTime();
    long getNonDrawTime();
    float getFPS();

    void setDebugTimerUsage( bool d );
    bool getDebugTimerUsage();

private:
    gravManager* grav;
    wxGLContext* glContext;
    DECLARE_EVENT_TABLE()

    void testDraw();
    void testKey( wxKeyEvent& evt );

    // tracks the aspect ratio of the screen for reshaping
    float screen_width, screen_height;

    // if draw is being called by a timer, have a reference to it so we can stop
    // it if need be
    RenderTimer* renderTimer;

    // for measuring the draw time
    wxStopWatch drawStopwatch;
    long lastDrawTime;
    long lastNonDrawTime;

    // for calculating averages
    long lastDrawTimeMax;
    long lastNonDrawTimeMax;
    long lastDrawTimeAvg;
    long lastNonDrawTimeAvg;
    int counter;
    int counterMax;

    wxStopWatch fpsStopwatch;
    int fpsCounter;
    float fpsResult;

    bool useDebugTimers;

};

#endif /*GLCANVAS_H_*/
