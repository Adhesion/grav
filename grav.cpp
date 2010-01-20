/**
 * @file grav.cpp
 * Implementation of main grav app behavior, like creating the gravManager &
 * GL canvas objects, as well as parsing command line arguments.
 * @author Andrew Ford
 */

#include "grav.h"
#include "GLCanvas.h"
#include "glutVideo.h"
#include "Earth.h"
#include "InputHandler.h"

#include <GL/glut.h>

IMPLEMENT_APP( gravApp )

bool gravApp::OnInit()
{   
    grav = new gravManager();
    
    frame = new wxFrame( (wxFrame*)NULL, -1, _("grav WX branch"),
                        wxPoint( 50, 50 ),
                        wxSize( grav->getWindowWidth(),
                                grav->getWindowHeight() ) );
    frame->Show( true );
    
    //wxBoxSizer* sizer = new wxBoxSizer( wxVERTICAL );
    
    int attribList[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 
                            0 };
    
    canvas = new GLCanvas( frame, grav, attribList );
    
    //sizer->Add( canvas, 1, wxEXPAND );
    //frame->SetSizer( sizer );
    //frame->SetAutoLayout( true );
    
    Timer* t = new Timer( canvas );
    t->Start();
    
    char* argv[1] = { "test" };
    int argc = 1;
    glutInit( &argc, argv );
    
    Earth* earth = new Earth();
    InputHandler* input = new InputHandler( earth, grav );
    
    // add the input handler to the chain of things that can handle
    // events & give the canvas focus so we don't have to click on it to
    // start sending key events
    canvas->PushEventHandler( input );
    canvas->SetFocus();
    
    grav->setEarth( earth );
    grav->setInput( input );
    grav->setBorderTex( "border.png" );
    bool res = grav->initSession( "224.2.224.225/20002", false );
    if ( res ) printf( "session initialized\n" );
    
    return true;
}
