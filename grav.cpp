/**
 * @file grav.cpp
 * Implementation of main grav app behavior, like creating the gravManager &
 * GL canvas objects, as well as parsing command line arguments.
 * @author Andrew Ford
 */

#include "Earth.h"
#include "grav.h"
#include "GLCanvas.h"
#include "gravManager.h"
#include "InputHandler.h"
#include "TreeControl.h"
#include "GLUtil.h"
#include "VideoSource.h"

#include <VPMedia/VPMLog.h>
#include <VPMedia/VPMPayloadDecoderFactory.h>

#include <GL/glut.h>

IMPLEMENT_APP( gravApp )

BEGIN_EVENT_TABLE(gravApp, wxApp)
EVT_IDLE(gravApp::idleHandler)
END_EVENT_TABLE()

bool gravApp::OnInit()
{
    grav = new gravManager();
    windowWidth = 900; windowHeight = 550; // defaults - should be command line
    // grav's windowwidth/height will be set by the glcanvas's resize callback
    
    parser.SetCmdLine( argc, argv );
    handleArgs();
    
    mainFrame = new wxFrame( (wxFrame*)NULL, -1, _("grav"),
                        wxPoint( 10, 50 ),
                        wxSize( windowWidth, windowHeight ) );
    mainFrame->Show( true );
    
    treeFrame = new wxFrame( (wxFrame*)NULL, -1, _("grav menu"),
                        wxPoint( 960, 50 ),
                        wxSize( 300, 600 ) );
    treeFrame->Show( true );
    
    int attribList[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 24,
                            0 };
    
    canvas = new GLCanvas( mainFrame, grav, attribList, windowWidth,
                            windowHeight );
    tree = new TreeControl( treeFrame );
    
    // initialize GL stuff (+ shaders) needs to be done AFTER attriblist is
    // used in making the canvas
    GLUtil::getInstance()->initGL();
    
    //printf( "hide root? %i\n", tree->HasFlag( wxTR_HIDE_ROOT ) );
    
    Timer* t = new Timer( canvas );
    t->Start();
    
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
    
    vpmlog_set_log_level( VPMLOG_LEVEL_DEBUG );
    
    mapRTP();
    
    if ( usingThreads )
        VPMthread = thread_start( threadTest, this );

    //
    //tree->addSession( std::string( "224.2.224.225/20002" ) );
    
    return true;
}

int gravApp::OnExit()
{
    // TODO: deconstructors, etc
    GLUtil::getInstance()->cleanupGL();
    return 0;
}

void gravApp::idleHandler( wxIdleEvent& evt )
{
    if ( !usingThreads )
        iterate();
}

void gravApp::iterate()
{
    grav->iterateSessions();
}

bool gravApp::handleArgs()
{
    parser.SetDesc( cmdLineDesc );
    int result = parser.Parse();
    
    // if parse returns -1 then it spit out the help message, so exit
    if ( result == -1 )
    {
        exit(0);
    }
    
    wxString videoAddress = parser.GetParam( 0 );
    bool res = grav->initSession( std::string((char*)videoAddress.char_str()),
                                    false );
    if ( res ) printf( "grav::video session initialized\n" );
    
    wxString audioAddress;
    if ( parser.Found( _("audio"), &audioAddress ) )
    {
        bool aRes = grav->initSession(
                            std::string((char*)audioAddress.char_str()), true );
        if ( aRes ) printf( "grav::audio session initialized\n" );
    }
    
    usingThreads = parser.Found( _("threads") );

    return true;
}

void gravApp::mapRTP()
{
    VPMPayloadDecoderFactory* decoderFactory =
                            VPMPayloadDecoderFactory::getInstance();
    decoderFactory->mapPayloadType( 45, "MPEG4" );
    decoderFactory->mapPayloadType( 96, "H264" );
    
    // map audio codecs
    decoderFactory->mapPayloadType( 122, "L16_8k_mono" );
    decoderFactory->mapPayloadType( 111, "L16_8k_stereo" );
    decoderFactory->mapPayloadType( 112, "L16_16k_mono" );
    decoderFactory->mapPayloadType( 113, "L16_16k_stereo" );
    decoderFactory->mapPayloadType( 114, "L16_32k_mono" );
    decoderFactory->mapPayloadType( 115, "L16_32k_stereo" );
    decoderFactory->mapPayloadType( 116, "L16_48k_mono" );
    decoderFactory->mapPayloadType( 117, "L16_48k_stereo" );
    
    // TODO: change these to mapPayloadType and add all PCM formats that rat
    // supports
    /*MAPSTATIC( RTP_PAYLOAD_MPEG4,
        VPM_PAYLOAD_VIDEO,
        "MPEG4",
        "video/mpeg4",
        "MPEG4",
        VPMMPEG4Decoder );

    MAPSTATIC( RTP_PAYLOAD_L16_48K_MONO,
        VPM_PAYLOAD_AUDIO,
        "L16_48k_mono",
        "audio/l16_48k_mono",
        "Linear16 48kHz Mono",
        VPMLinear16Decoder_48k_mono );
        
    MAPSTATIC( RTP_PAYLOAD_L16_48K_STEREO,
        VPM_PAYLOAD_AUDIO,
        "L16_48k_stereo",
        "audio/l16_48k_stereo",
        "Linear16 48kHz STEREO",
        VPMLinear16Decoder_48k_stereo );
        
    MAPSTATIC( RTP_PAYLOAD_L16_16K_MONO,
        VPM_PAYLOAD_AUDIO,
        "L16_16k_mono",
        "audio/l16_16k_mono",
        "Linear16 16kHz Mono",
        VPMLinear16Decoder_16k_mono );
        
    MAPSTATIC( RTP_PAYLOAD_L16_16K_STEREO,
        VPM_PAYLOAD_AUDIO,
        "L16_16k_stereo",
        "audio/l16_16k_stereo",
        "Linear16 16kHz STEREO",
        VPMLinear16Decoder_16k_stereo );*/
}

void* gravApp::threadTest( void* args )
{
    printf( "gravApp::starting thread...\n" );
    gravApp* g = (gravApp*)args;
    sleep( 1 );
    while ( true )
    {
        usleep( 16000 );
        g->iterate();
    }
    return 0;
}
