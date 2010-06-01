/**
 * @file GLCanvas.h
 * Implementation of the GL canvas class. Also defines a timer for defining
 * when to render.
 * @author Andrew Ford
 */

#include "gravManager.h"
#include "GLCanvas.h"
#include "InputHandler.h"

#include <GL/glut.h>

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
    gettimeofday( &time, NULL );
    lastTimeMS = time.tv_usec;
}

void GLCanvas::handlePaintEvent( wxPaintEvent& evt )
{
    draw();
}

void GLCanvas::draw()
{
    gettimeofday( &time, NULL );
    time_t diff = time.tv_usec - lastTimeMS;
    lastTimeMS = time.tv_usec;

    //if ( diff > 30000 )
    //    printf( "GLCanvas::draw: diff is %d\n", diff );

    if( !IsShown() ) return;
    
    //printf( "drawing\n" );
    SetCurrent( *glContext );
    wxPaintDC( this );
    
    if ( grav != NULL )
        grav->draw();
    //testDraw();
    
    SwapBuffers();
}

void GLCanvas::resize( wxSizeEvent& evt )
{
    printf( "resize callback: to %ix%i\n", evt.GetSize().GetWidth(),
                                            evt.GetSize().GetHeight() );
    OnSize( evt );
    Refresh( false );
    GLreshape( evt.GetSize().GetWidth(), evt.GetSize().GetHeight() );
}

void GLCanvas::testDraw()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, (GLint)GetSize().x, (GLint)GetSize().y);
 
    glBegin(GL_POLYGON);
        glColor3f(1.0, 1.0, 1.0);
        glVertex2f(-0.5, -0.5);
        glVertex2f(-0.5, 0.5);
        glVertex2f(0.5, 0.5);
        glVertex2f(0.5, -0.5);
        glColor3f(0.4, 0.5, 0.4);
        glVertex2f(0.0, -0.8);
    glEnd();
 
    glBegin(GL_POLYGON);
        glColor3f(1.0, 0.0, 0.0);
        glVertex2f(0.1, 0.1);
        glVertex2f(-0.1, 0.1);
        glVertex2f(-0.1, -0.1);
        glVertex2f(0.1, -0.1);
    glEnd();
 
// using a little of glut
    glColor4f(0,0,1,1);
    glutWireTeapot(0.4);
 
    glLoadIdentity();
    glColor4f(2,0,1,1);
    glutWireTeapot(0.6);
// done using glut
 
    glFlush();
    
}

void GLCanvas::testKey( wxKeyEvent& evt )
{
    printf( "GLCanvas::testKey: pressed key %c\n",
            (unsigned char)evt.GetKeyCode() );
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

Timer::Timer( GLCanvas* c ) :
    canvas( c )
{
    
}

void Timer::Notify()
{
    canvas->draw();
}

void Timer::Start()
{
    //wxTimer::Start( 16 );
}
