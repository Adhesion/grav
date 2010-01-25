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
#include "TreeControl.h"

#include <GL/glut.h>

IMPLEMENT_APP( gravApp )

bool gravApp::OnInit()
{   
    grav = new gravManager();
    
    mainFrame = new wxFrame( (wxFrame*)NULL, -1, _("grav WX branch"),
                        wxPoint( 10, 50 ),
                        wxSize( grav->getWindowWidth(),
                                grav->getWindowHeight() ) );
    mainFrame->Show( true );
    
    treeFrame = new wxFrame( (wxFrame*)NULL, -1, _("grav menu"),
                        wxPoint( 860, 50 ),
                        wxSize( 300, 700 ) );
    treeFrame->Show( true );
    
    int attribList[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 
                            0 };
    
    canvas = new GLCanvas( mainFrame, grav, attribList );
    tree = new TreeControl( treeFrame, grav );
    
    printf( "hide root? %i\n", tree->HasFlag( wxTR_HIDE_ROOT ) );
    
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
    grav->setTree( tree );
    grav->setBorderTex( "border.png" );
    
    bool res = grav->initSession( "224.2.224.225/20002", false );
    if ( res ) printf( "grav::session initialized\n" );
    //tree->addSession( std::string( "224.2.224.225/20002" ) );
    
    return true;
}
