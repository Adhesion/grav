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
#include "VideoListener.h"
#include "AudioManager.h"
#include "Frame.h"

#include <VPMedia/VPMLog.h>
#include <VPMedia/VPMPayloadDecoderFactory.h>
#include <VPMedia/VPMSessionFactory.h>
#include <VPMedia/random_helper.h>

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

    sf = VPMSessionFactory::getInstance();
    videoSession_listener = new VideoListener( grav );
    audioSession_listener = new AudioManager();
    videoInitialized = false; audioInitialized = false;

    handleArgs();
    
    mainFrame = new Frame( (wxFrame*)NULL, -1, _("grav"), wxPoint( 10, 50 ),
                        wxSize( windowWidth, windowHeight ) );
    mainFrame->Show( true );
    mainFrame->SetName( _("main grav frame") );
    SetTopWindow( mainFrame );
    mainFrame->SetFocus();
    //mainFrame->ShowFullScreen( true );
    
    treeFrame = new Frame( mainFrame, -1, _("grav menu"), wxPoint( 960, 50 ),
                        wxSize( 300, 600 ) );
    treeFrame->Show( true );
    treeFrame->SetName( _("tree frame") );
    
    int attribList[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 24,
                            0 };
    
    canvas = new GLCanvas( mainFrame, grav, attribList, windowWidth,
                            windowHeight );
    tree = new TreeControl( treeFrame );
    tree->setSourceManager( grav );
    
    // since that bool is used in init, set it before init
    GLUtil::getInstance()->setShaderDisable( disableShaders );

    // initialize GL stuff (+ shaders) needs to be done AFTER attriblist is
    // used in making the canvas
    GLUtil::getInstance()->initGL();
    
    //printf( "hide root? %i\n", tree->HasFlag( wxTR_HIDE_ROOT ) );
    timer = new Timer( canvas, timerInterval );
    timer->Start();
    videoSession_listener->setTimer( timer );
    
    Earth* earth = new Earth();
    InputHandler* input = new InputHandler( earth, grav, mainFrame );
    
    // add the input handler to the chain of things that can handle
    // events & give the canvas focus so we don't have to click on it to
    // start sending key events
    canvas->PushEventHandler( input );
    canvas->SetFocus();
    canvas->setTimer( timer );
    
    grav->setEarth( earth );
    grav->setInput( input );
    grav->setTree( tree );
    grav->setBorderTex( "border.png" );
    
    vpmlog_set_log_level( VPMLOG_LEVEL_DEBUG );
    
    mapRTP();
    
    if ( usingThreads )
    {
        grav->setThreads( usingThreads );
        threadRunning = true;
        VPMthread = thread_start( threadTest, this );
    }

    //
    //tree->addSession( std::string( "224.2.224.225/20002" ) );
    
    return true;
}

int gravApp::OnExit()
{
    printf( "grav::Exiting...\n" );
    // TODO: test this stuff more, valgrind etc

    if ( usingThreads )
    {
        threadRunning = false;
        thread_join( VPMthread );
    }

    // note, tree and canvas get deleted automatically since they're children
    // of frames and frames delete their children automatically
    // and those set the grav manager's tree to null and stop the timer
    // respectively
    delete timer;

    if ( videoSession && videoInitialized )
    {
        delete videoSession;
        delete videoSession_listener;
    }
    if ( audioEnabled && audioSession && audioInitialized )
    {
        delete audioSession;
        delete audioSession_listener;
    }

    delete grav;

    GLUtil::getInstance()->cleanupGL();

    return 0;
}

void gravApp::idleHandler( wxIdleEvent& evt )
{
    if ( !usingThreads )
    {
        iterateSessions();
        usleep( 500 );
        evt.RequestMore();
    }

    // note this is the method for rendering on idle, with a limiter to 16ms
    // (60fps) - disabled for now since timer seems decent enough
    /*unsigned long time = (unsigned long)timer->getTiming();
    if ( time > 16000 )
    {
        printf( "%lu\n", time );
        //canvas->draw();
        timer->resetTiming();
    }
    else
    {
        usleep( 1000 );
    }

    evt.RequestMore();*/
}

bool gravApp::initSession( std::string address, bool audio )
{
    if ( !audio )
    {
        videoSession = sf->createSession( address.c_str(),
                                        *videoSession_listener );

        videoSession->enableVideo(true);
        videoSession->enableAudio(false);
        videoSession->enableOther(false);

        if ( !videoSession->initialise() ) {
            fprintf( stderr, "error: failed to initialise session\n" );
            return false;
        }

        videoSession_ts = random32();
        videoInitialized = true;
    }

    else
    {
        audioSession = sf->createSession( address.c_str(),
                                        *audioSession_listener);

        audioSession->enableVideo(false);
        audioSession->enableAudio(true);
        audioSession->enableOther(false);

        if (!audioSession->initialise()) {
            fprintf(stderr, "error: failed to initialise audioSession\n");
            return false;
        }

        audioSession_ts = random32();
        audioInitialized = true;
    }

    return true;
}

void gravApp::iterateSessions()
{
    if ( videoSession && videoInitialized )
        videoSession->iterate( videoSession_ts++ );
    if ( audioEnabled && audioSession && audioInitialized )
        audioSession->iterate( audioSession_ts++ );
}

bool gravApp::handleArgs()
{
    parser.SetDesc( cmdLineDesc );
    int result = parser.Parse();
    
    // if parse returns -1 then it spit out the help message, so exit
    if ( result == -1 )
    {
        Exit();
    }
    
    wxString videoAddress = parser.GetParam( 0 );
    bool res = initSession( std::string((char*)videoAddress.char_str()),
                                    false );
    if ( res ) printf( "grav::video session initialized\n" );
    
    wxString audioAddress;
    if ( parser.Found( _("audio"), &audioAddress ) )
    {
        bool aRes = initSession(
                            std::string((char*)audioAddress.char_str()), true );
        if ( aRes )
        {
            printf( "grav::audio session initialized\n" );
            grav->setAudio( audioSession_listener );
            audioEnabled = true;
        }
    }

    usingThreads = parser.Found( _("threads") );

    disableShaders = parser.Found( _("disable-shaders") );

    grav->setRunwayUsage( !parser.Found( _("automatic") ) );

    long int fps;
    if ( parser.Found( _("fps"), &fps ) )
    {
        timerInterval = floor( 1000.0f / (float)fps );
        if ( timerInterval < 1 )
        {
            printf( "ERROR: invalid fps value, resetting to ~60\n" );
            timerInterval = 16;
        }
    }
    else
    {
        timerInterval = 16;
    }

    return true;
}

void gravApp::mapRTP()
{
    VPMPayloadDecoderFactory* decoderFactory =
                            VPMPayloadDecoderFactory::getInstance();
    decoderFactory->mapPayloadType( 45, "MPEG4" );
    decoderFactory->mapPayloadType( 96, "H264" );
    
    decoderFactory->mapPayloadType( 77, "H261AS" );

    // map audio codecs
    decoderFactory->mapPayloadType( 122, "L16_8k_mono" );
    decoderFactory->mapPayloadType( 111, "L16_8k_stereo" );
    decoderFactory->mapPayloadType( 112, "L16_16k_mono" );
    decoderFactory->mapPayloadType( 113, "L16_16k_stereo" );
    decoderFactory->mapPayloadType( 114, "L16_32k_mono" );
    decoderFactory->mapPayloadType( 115, "L16_32k_stereo" );
    decoderFactory->mapPayloadType( 116, "L16_48k_mono" );
    decoderFactory->mapPayloadType( 117, "L16_48k_stereo" );
}

void* gravApp::threadTest( void* args )
{
    printf( "gravApp::starting network/decoding thread...\n" );
    gravApp* g = (gravApp*)args;
    // wait a bit before starting this thread, since doing it too early might
    // affect the WX tree before it's fully initialized somehow, rarely
    // resulting in broken text or a crash
    usleep( 100000 );
    while ( g->isThreadRunning() )
    {
        g->iterateSessions();
        usleep( 1000 );
    }
    printf( "gravApp::thread ending...\n" );
    return 0;
}

bool gravApp::isThreadRunning()
{
    return threadRunning;
}
