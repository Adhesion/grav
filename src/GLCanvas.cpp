/**
 * @file GLCanvas.h
 * Implementation of the GL canvas class. Also defines a timer for defining
 * when to render.
 * @author Andrew Ford
 */

#include "gravManager.h"
#include "GLCanvas.h"
#include "InputHandler.h"
#include "Timers.h"

BEGIN_EVENT_TABLE(GLCanvas, wxGLCanvas)
EVT_PAINT(GLCanvas::handlePaintEvent)
EVT_SIZE(GLCanvas::resize)
END_EVENT_TABLE()

GLCanvas::GLCanvas( wxWindow* parent, gravManager* g, int* attributes,
                        int width, int height ) :
    wxGLCanvas( parent, wxID_ANY, attributes, wxDefaultPosition,
                wxDefaultSize, 0, wxT("grav GLCanvas")), grav( g )
{
    SetSize( wxSize( width, height ) );
    glContext = new wxGLContext( this );
    SetCurrent( *glContext );

    lastDrawTime = 0;
    lastNonDrawTime = 0;
    lastDrawTimeMax = 0;
    lastNonDrawTimeMax = 0;
    counter = 0;
    counterMax = 5;

    fpsCounter = 0;
    fpsResult = 0;

    useDebugTimers = false;
}

GLCanvas::~GLCanvas()
{
    stopTimer();
}

void GLCanvas::handlePaintEvent( wxPaintEvent& evt )
{
    draw();
}

void GLCanvas::draw()
{
    if ( useDebugTimers )
    {
        lastNonDrawTime = drawStopwatch.Time();
        lastNonDrawTimeMax += lastNonDrawTime;
        drawStopwatch.Start();
    }

    if( !IsShown() ) return;

    SetCurrent( *glContext );
    wxPaintDC( this );

    if ( grav != NULL )
        grav->draw();

    SwapBuffers();

    if ( useDebugTimers )
    {
        lastDrawTime = drawStopwatch.Time();
        lastDrawTimeMax += lastDrawTime;
        drawStopwatch.Start();

        counter = (counter+1) % counterMax;
        if ( counter == 0 )
        {
            lastDrawTimeAvg = lastDrawTimeMax / (float)counterMax;
            lastNonDrawTimeAvg = lastNonDrawTimeMax / (float)counterMax;
            lastDrawTimeMax = 0;
            lastNonDrawTimeMax = 0;
        }

        fpsCounter++;
        long elapsed = fpsStopwatch.Time();
        if ( elapsed > 500 )
        {
            fpsResult = (float)fpsCounter * 1000.0f / (float)elapsed;
            fpsCounter = 0;
            fpsStopwatch.Start();
        }
    }
}

void GLCanvas::resize( wxSizeEvent& evt )
{
    printf( "resize callback: to %ix%i\n", evt.GetSize().GetWidth(),
                                            evt.GetSize().GetHeight() );
    OnSize( evt );
    Refresh( false );
    GLreshape( evt.GetSize().GetWidth(), evt.GetSize().GetHeight() );
}

void GLCanvas::GLreshape( int w, int h )
{
    glViewport(0, 0, w, h);

    if (w > h)
    {
        screen_height = 1.0;
        screen_width = (float)w/(float)h;
    }
    else
    {
        screen_height = (float)h/(float)w;
        screen_width = 1.0;
    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-screen_width/10.0, screen_width/10.0,
              -screen_height/10.0, screen_height/10.0,
              0.1, 50.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(grav->getCamX(), grav->getCamY(), grav->getCamZ(),
              0.0, 0.0, -25.0,
              0.0, 1.0, 0.0);

    // note this should be done last since stuff inside setwindowsize
    // (finding the world space bounds for the screen) depends on the matrices
    // above being accurate
    grav->setWindowSize( w, h );
}

void GLCanvas::stopTimer()
{
    renderTimer->Stop();
}

void GLCanvas::setTimer( RenderTimer* t )
{
    renderTimer = t;
}

long GLCanvas::getDrawTime()
{
    return lastDrawTimeAvg;
}

long GLCanvas::getNonDrawTime()
{
    return lastNonDrawTimeAvg;
}

float GLCanvas::getFPS()
{
    return fpsResult;
}

void GLCanvas::setDebugTimerUsage( bool d )
{
    useDebugTimers = d;
    if ( d )
        fpsStopwatch.Start();
    else
    {
        fpsStopwatch.Pause();
        fpsCounter = 0;
    }
}

bool GLCanvas::getDebugTimerUsage()
{
    return useDebugTimers;
}
