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
    
    int attribList[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 
                            0 };
    
    canvas = new GLCanvas( frame, grav, attribList );
    Timer* t = new Timer( canvas );
    t->Start();
    
    /*char* argv[1] = { "test" };
    int argc = 1;
    glutInit( &argc, argv );*/
    
    Earth* earth = new Earth();
    InputHandler* input = new InputHandler( grav->getSources(),
                                            grav->getDrawnObjects(),
                                            grav->getSelectedObjects(),
                                            grav->getSiteIDGroups(),
                                            earth, grav );
    grav->setEarth( earth );
    grav->setInput( input );
    grav->initSession( "224.2.224.225/20002", false );
    
    return true;
}
