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

    drawStopWatch.Start();
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
    lastNonDrawTime = drawStopWatch.Time();
    drawStopWatch.Start();

    if( !IsShown() ) return;
    
    SetCurrent( *glContext );
    wxPaintDC( this );
    
    if ( grav != NULL )
        grav->draw();
    
    SwapBuffers();

    lastDrawTime = drawStopWatch.Time();
    drawStopWatch.Start();
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
    return lastDrawTime;
}

long GLCanvas::getNonDrawTime()
{
    return lastNonDrawTime;
}
