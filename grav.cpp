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
#include "SessionTreeControl.h"
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
END_EVENT_TABLE(); // this ; is not necessary, just makes eclipse's syntax
                   // parser shut up

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

    if ( !handleArgs() )
    {
        delete grav;
        return false;
    }

    // GUI setup
    mainFrame = new Frame( (wxFrame*)NULL, -1, _("grav"), wxPoint( 10, 50 ),
                        wxSize( windowWidth, windowHeight ) );
    mainFrame->Show( true );
    mainFrame->SetName( _("main grav frame") );
    SetTopWindow( mainFrame );
    if ( startFullscreen )
        mainFrame->ShowFullScreen( true );

    int treeX = 960; int treeY = 50;
    // forces resize - for some reason it doesn't draw inner contents until
    // resized
    int treeWidth = 251; int treeHeight = 501;
    treeFrame = new Frame( mainFrame, -1, _("grav menu"),
                            wxPoint( treeX, treeY ),
                            wxSize( treeWidth, treeHeight ) );
    treeFrame->Show( true );
    treeFrame->SetSizeHints( 250, 500, 250, 500 );

    treePanel = new wxPanel( treeFrame, wxID_ANY, treeFrame->GetPosition(),
                                treeFrame->GetSize() );
    treeNotebook = new wxNotebook( treePanel, wxID_ANY,
                               treePanel->GetPosition(), treePanel->GetSize() );

    int attribList[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 24,
                            0 };

    canvas = new GLCanvas( mainFrame, grav, attribList, windowWidth,
                            windowHeight );
    sourceTree = new TreeControl( treeNotebook );
    sourceTree->setSourceManager( grav );
    sessionTree = new SessionTreeControl( treeNotebook );
    treeNotebook->AddPage( sourceTree, _("Videos"), true );
    treeNotebook->AddPage( sessionTree, _("Sessions") );
    // sizer for the notebook in general
    wxBoxSizer* treeSizer = new wxBoxSizer( wxVERTICAL );
    treePanel->SetSizer( treeSizer );
    treeSizer->Fit( treeFrame );
    treeSizer->Add( treeNotebook, wxEXPAND );
    treeSizer->Show( treeNotebook );
    treeSizer->Layout();

    // put the main frame on top
    mainFrame->Raise();

    // since that bool is used in init, set it before init
    GLUtil::getInstance()->setShaderEnable( enableShaders );

    // initialize GL stuff (+ shaders) needs to be done AFTER attriblist is
    // used in making the canvas
    GLUtil::getInstance()->initGL();

    grav->setHeaderString( "RIT Global Collaboration Grid" );
    
    //printf( "hide root? %i\n", tree->HasFlag( wxTR_HIDE_ROOT ) );
    timer = new Timer( canvas, timerInterval );
    //timer->Start();
    //wxStopWatch* t2 = new wxStopWatch();
    //videoSession_listener->setTimer( t2 );
    
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
    grav->setTree( sourceTree );
    grav->setBorderTex( "border.png" );
    
    vpmlog_set_log_level( VPMLOG_LEVEL_DEBUG );
    
    mapRTP();

    for ( unsigned int i = 0; i < initialVideoAddresses.size(); i++ )
    {
        initSession( initialVideoAddresses[i], false );
    }
    for ( unsigned int i = 0; i < initialAudioAddresses.size(); i++ )
    {
        initSession( initialAudioAddresses[i], true );
    }

    printf( "grav:init function complete\n" );
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

    if ( videoInitialized )
    {
        for ( unsigned int i = 0; i < videoSessions.size(); i++ )
            delete videoSessions[i];
        delete videoSession_listener;
    }
    if ( audioEnabled && audioInitialized )
    {
        for ( unsigned int i = 0; i < audioSessions.size(); i++ )
            delete audioSessions[i];
        delete audioSession_listener;
    }

    delete grav;

    GLUtil::getInstance()->cleanupGL();

    return 0;
}

void gravApp::idleHandler( wxIdleEvent& evt )
{
    if ( usingThreads && !threadRunning )
    {
        grav->setThreads( usingThreads );
        threadRunning = true;
        VPMthread = thread_start( threadTest, this );
    }

    if ( !usingThreads )
        iterateSessions();

    // note this is the method for rendering on idle, with a limiter to 16ms
    // (60fps)
    unsigned long time = (unsigned long)timer->getTiming();
    if ( time > (unsigned long)timerIntervalUS )
    {
        //printf( "%lu\n", time );
        canvas->draw();
        timer->resetTiming();
    }
    else
    {
        wxMilliSleep( 1 );
    }

    evt.RequestMore();
}

bool gravApp::initSession( std::string address, bool audio )
{
    if ( !audio )
    {
        VPMSession* videoSession = sf->createSession( address.c_str(),
                                        *videoSession_listener );

        videoSession->enableVideo(true);
        videoSession->enableAudio(false);
        videoSession->enableOther(false);

        if ( !videoSession->initialise() ) {
            fprintf( stderr, "error: failed to initialise session\n" );
            return false;
        }

        videoSession_ts = random32();
        videoSessions.push_back( videoSession );
        videoInitialized = true;
        printf( "grav::initialized video session on %s\n", address.c_str() );
    }
    else
    {
        VPMSession* audioSession = sf->createSession( address.c_str(),
                                        *audioSession_listener);

        audioSession->enableVideo(false);
        audioSession->enableAudio(true);
        audioSession->enableOther(false);

        if (!audioSession->initialise()) {
            fprintf(stderr, "error: failed to initialise audioSession\n");
            return false;
        }

        audioSession_ts = random32();
        audioSessions.push_back( audioSession );
        audioInitialized = true;
        audioEnabled = true;
        printf( "grav::initialized audio session on %s\n", address.c_str() );
        grav->setAudio( audioSession_listener );
    }

    sessionTree->addSession( address, audio );
    return true;
}

void gravApp::iterateSessions()
{
    bool haveSessions = false;
    if ( videoInitialized )
    {
        for ( unsigned int i = 0; i < videoSessions.size(); i++ )
        {
            videoSessions[i]->iterate( videoSession_ts++ );
            haveSessions = haveSessions || true;
        }
    }
    if ( audioEnabled && audioInitialized )
    {
        for ( unsigned int i = 0; i < audioSessions.size(); i++ )
        {
            audioSessions[i]->iterate( audioSession_ts++ );
            haveSessions = haveSessions || true;
        }
    }
    // if there are no sessions, sleep so as not to spin and consume CPU
    // needlessly
    if ( !haveSessions )
        wxMicroSleep( 500 );
    //if ( videoSession && videoInitialized )
    //    videoSession->iterate( videoSession_ts++ );
    //if ( audioEnabled && audioSession && audioInitialized )
    //    audioSession->iterate( audioSession_ts++ );
}

bool gravApp::handleArgs()
{
    parser.SetDesc( cmdLineDesc );
    int result = parser.Parse();
    
    // if parse returns -1 then it spit out the help message, so exit
    if ( result == -1 )
        return false;
    
    wxString videoAddress = parser.GetParam( 0 );
    initialVideoAddresses.push_back(
                                std::string((char*)videoAddress.char_str()) );
    
    wxString audioAddress;
    if ( parser.Found( _("audio"), &audioAddress ) )
    {
        initialAudioAddresses.push_back(
                                std::string((char*)audioAddress.char_str()) );
    }

    usingThreads = parser.Found( _("threads") );

    enableShaders = parser.Found( _("enable-shaders") );

    startFullscreen = parser.Found( _("fullscreen") );

    grav->setAutoFocusRotate( parser.Found( _("automatic") ) );

    grav->setGridAuto( parser.Found( _("gridauto") ) );

    long int fps;
    if ( parser.Found( _("fps"), &fps ) )
    {
        timerInterval = floor( 1000.0f / (float)fps );
        timerIntervalUS = floor( 1000000.0f / (float)fps );
        if ( timerInterval < 1 )
        {
            printf( "ERROR: invalid fps value, resetting to ~60\n" );
            timerInterval = 16;
            timerIntervalUS = 16667;
        }
    }
    else
    {
        timerInterval = 16;
        timerIntervalUS = 16667;
    }
    printf( "microsecond timer is %i\n", timerIntervalUS );

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
    wxMilliSleep( 100 );
    while ( g->isThreadRunning() )
    {
        g->iterateSessions();
    }
    printf( "gravApp::thread ending...\n" );
    return 0;
}

bool gravApp::isThreadRunning()
{
    return threadRunning;
}
