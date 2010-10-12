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
#include "SessionManager.h"
#include "GLUtil.h"
#include "VideoSource.h"
#include "VideoListener.h"
#include "AudioManager.h"
#include "Frame.h"

#include <VPMedia/VPMLog.h>
#include <VPMedia/VPMPayloadDecoderFactory.h>
#include <VPMedia/VPMSessionFactory.h>

IMPLEMENT_APP( gravApp )

BEGIN_EVENT_TABLE(gravApp, wxApp)
EVT_IDLE(gravApp::idleHandler)
END_EVENT_TABLE(); // this ; is not necessary, just makes eclipse's syntax
                   // parser shut up

bool gravApp::threadDebug = false;
int gravApp::threadCounter = 0;

bool gravApp::OnInit()
{
    grav = new gravManager();
    windowWidth = 900; windowHeight = 550; // defaults - should be command line
    // grav's windowwidth/height will be set by the glcanvas's resize callback
    
    parser.SetCmdLine( argc, argv );

    videoSessionListener = new VideoListener( grav );
    audioSessionListener = new AudioManager();
    sessionManager = new SessionManager( videoSessionListener,
                                            audioSessionListener );
    //videoInitialized = false; audioInitialized = false;

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
    mainFrame->setSourceManager( grav );
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

    if ( headerSet )
        grav->setHeaderString( header );
    else
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

    sessionTree->setSessionManager( sessionManager );
    for ( unsigned int i = 0; i < initialVideoAddresses.size(); i++ )
    {
        printf ( "grav::initializing video address %s\n",
                    initialVideoAddresses[i].c_str() );
        sessionTree->addSession( initialVideoAddresses[i], false, false );
    }
    for ( unsigned int i = 0; i < initialAudioAddresses.size(); i++ )
    {
        printf ( "grav::initializing audio address %s\n",
                    initialAudioAddresses[i].c_str() );
        sessionTree->addSession( initialAudioAddresses[i], true, false );
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

    delete sessionManager;
    delete videoSessionListener;
    delete audioSessionListener;
    /*if ( videoInitialized )
    {
        for ( unsigned int i = 0; i < videoSessions.size(); i++ )
            delete videoSessions[i];

    }
    if ( audioEnabled && audioInitialized )
    {
        for ( unsigned int i = 0; i < audioSessions.size(); i++ )
            delete audioSessions[i];

    }*/

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
        sessionManager->iterateSessions();

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

void gravApp::iterateSessions()
{
    bool haveSessions = sessionManager->iterateSessions();

    // if there are no sessions, sleep so as not to spin and consume CPU
    // needlessly
    if ( !haveSessions )
        wxMicroSleep( 500 );
}

bool gravApp::handleArgs()
{
    parser.SetDesc( cmdLineDesc );
    int result = parser.Parse();

    // if parse returns -1 then it spit out the help message, so exit
    if ( result == -1 )
        return false;

    for ( unsigned int i = 0; i < parser.GetParamCount(); i++ )
    {
        wxString videoAddress = parser.GetParam( i );
        initialVideoAddresses.push_back(
                                  std::string((char*)videoAddress.char_str()) );
    }

    wxString audioAddress;
    if ( parser.Found( _("audio"), &audioAddress ) )
    {
        initialAudioAddresses.push_back(
                                std::string((char*)audioAddress.char_str()) );
        grav->setAudio( audioSessionListener );
    }

    wxString headerWX;
    headerSet = parser.Found( _("header"), &headerWX );
    if ( headerSet )
    {
        header = std::string((char*)headerWX.char_str());
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
    printf( "grav::starting network/decoding thread...\n" );
    gravApp* g = (gravApp*)args;
    // wait a bit before starting this thread, since doing it too early might
    // affect the WX tree before it's fully initialized somehow, rarely
    // resulting in broken text or a crash
    wxMilliSleep( 100 );
    while ( g->isThreadRunning() )
    {
        g->iterateSessions();
        if ( threadDebug )
        {
            if ( threadCounter == 0 )
                printf( "grav::thread still running\n" );
            threadCounter = (threadCounter+1)%1000;
        }
    }
    printf( "grav::thread ending...\n" );
    return 0;
}

bool gravApp::isThreadRunning()
{
    return threadRunning;
}
