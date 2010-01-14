/**
 * @file GLCanvas.h
 * Implementation of the GL canvas class. Also defines a timer for defining
 * when to render.
 * @author Andrew Ford
 */

#include "GLCanvas.h"
#include "glutVideo.h"

BEGIN_EVENT_TABLE(GLCanvas, wxGLCanvas)
EVT_PAINT(GLCanvas::draw)
END_EVENT_TABLE()

GLCanvas::GLCanvas( wxWindow* parent, gravManager* g, int* attributes ) :
    wxGLCanvas( parent, wxID_ANY, attributes, wxDefaultPosition, wxDefaultSize,
                    0, wxT("grav GLCanvas")), grav( g )
{
    glContext = new wxGLContext( this );
    wxGLCanvas::SetCurrent( *glContext );
}

void GLCanvas::draw( wxPaintEvent& evt )
{
    if( !IsShown() ) return;
    
    printf( "drawing\n" );
    wxGLCanvas::SetCurrent( *glContext );
    wxPaintDC( this );
    
    grav->draw();
    
    SwapBuffers();
}

Timer::Timer( GLCanvas* c ) :
    canvas( c )
{
    
}

void Timer::Notify()
{
    canvas->Refresh( false );
}

void Timer::Start()
{
    wxTimer::Start( 30 );
}
