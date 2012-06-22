/*
 * @file grav.cpp
 *
 * Implementation of main grav app behavior, like creating the ObjectManager &
 * GL canvas objects, as well as parsing command line arguments.
 *
 * @author Andrew Ford
 * Copyright (C) 2011 Rochester Institute of Technology
 *
 * This file is part of grav.
 *
 * grav is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * grav is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with grav.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "grav.h"
#include "gravUtil.h"
#include "Earth.h"
#include "GLCanvas.h"
#include "ObjectManager.h"
#include "InputHandler.h"
#include "TreeControl.h"
#include "SessionTreeControl.h"
#include "SessionManager.h"
#include "GLUtil.h"
#include "VideoSource.h"
#include "VideoListener.h"
#include "AudioManager.h"
#include "Frame.h"
#include "SideFrame.h"
#include "Timers.h"
#include "VenueClientController.h"

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
    parser.SetCmdLine( argc, argv );

    if ( !handleArgs() )
    {
        return false;
    }

    // Some weirdness happens if this is called before arg handling, etc.
    gravUtil::initLogging();

    objectMan = new ObjectManager();
    // defaults - can be changed by command line
    windowWidth = 900; windowHeight = 550;
    startX = 10; startY = 50;
    // ObjectManager's windowwidth/height will be set by the glcanvas's resize
    // callback

    videoSessionListener = new VideoListener( objectMan );
    audioSessionListener = new AudioManager();
    sessionManager = new SessionManager( videoSessionListener,
                                            audioSessionListener, objectMan );
    // video session listener needs to have ref to session manager to figure out
    // VPMSession -> SessionEntry
    videoSessionListener->setSessionManager( sessionManager );

    // Set verbosity here, nothing should use gravUtil::logVerbose before this.
    if ( verbose )
        wxLog::SetVerbose( true );
    if ( VPMverbose )
        vpmlog_set_log_level( VPMLOG_LEVEL_DEBUG );

#ifdef VPMEDIA_HAVE_FFMPEG
    if ( VPMverbose )
        av_log_set_level( AV_LOG_VERBOSE );
    else
        av_log_set_level( AV_LOG_FATAL );
#endif

    // GUI setup
    mainFrame = new Frame( (wxFrame*)NULL, -1, _("grav"),
                        wxPoint( startX, startY ),
                        wxSize( windowWidth, windowHeight ) );
    mainFrame->Show( true );
    mainFrame->SetName( _("main grav frame") );
    mainFrame->setObjectManager( objectMan );
    SetTopWindow( mainFrame );
    if ( startFullscreen )
        mainFrame->ShowFullScreen( true );

    int treeX = 960; int treeY = 50;
    // forces resize - for some reason it doesn't draw inner contents until
    // resized
    int treeWidth = 251; int treeHeight = 501;
    treeFrame = new SideFrame( mainFrame, -1, _("grav menu"),
                            wxPoint( treeX, treeY ),
                            wxSize( treeWidth, treeHeight ) );
    treeFrame->Show( true );
    treeFrame->SetSizeHints( 250, 500, 250, 500 );

    std::string iconLoc = gravUtil::getInstance()->findFile( "grav-icon.xpm" );

    if ( iconLoc.compare( "" ) != 0 )
    {
        wxIcon mainIcon( wxString( iconLoc.c_str(), wxConvUTF8 ),
                wxBITMAP_TYPE_XPM );
        wxIcon treeIcon( wxString( iconLoc.c_str(), wxConvUTF8 ),
                wxBITMAP_TYPE_XPM );

        mainFrame->SetIcon( mainIcon );
        treeFrame->SetIcon( treeIcon );
    }

    treePanel = new wxPanel( treeFrame, wxID_ANY, treeFrame->GetPosition(),
                                treeFrame->GetSize() );
    treeNotebook = new wxNotebook( treePanel, wxID_ANY,
                               treePanel->GetPosition(), treePanel->GetSize() );

    int attribList[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 24,
                            0 };

    canvas = new GLCanvas( mainFrame, objectMan, attribList,
                            mainFrame->GetClientSize() );
    sourceTree = new TreeControl( treeNotebook );
    sourceTree->setObjectManager( objectMan );
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

    // log here instead of in handleargs, see above/in handleargs
    // (handleargs is where the timer intervals actually get set)
    // might be that we can't do logging until main window is created
    if ( fps > 1000 )
    {
        gravUtil::logVerbose( "grav::warning: invalid fps value %li, "
                              "reset to 60\n", fps );
        // note the "fps" variable here is just the input from the command line,
        // the timer interval - what actually determines the rendering timing -
        // has already been reset to 16ms in handleArgs
    }

    if ( printVersion )
    {
        std::string ver = "grav ";
        ver += gravUtil::getVersionString();
        gravUtil::logMessage( ver.c_str() );
    }

    if ( disablePython )
    {
        PythonTools::disableInit = true;
    }

    // since these bools are used in glinit, set them before glinit
    GLUtil::getInstance()->setShaderEnable( enableShaders );
    GLUtil::getInstance()->setBufferFontUsage( bufferFont );

    // initialize GL stuff (+ shaders) needs to be done AFTER attriblist is
    // used in making the canvas
    if ( !GLUtil::getInstance()->initGL() )
    {
        // bail out if something failed
        gravUtil::logError( "grav::OnInit(): ERROR: initGL() failed, "
                "exiting\n" );
        delete sessionManager;
        delete videoSessionListener;
        delete audioSessionListener;
        delete objectMan;
        return false;
    }

    GLUtil::getInstance()->addTexture( "border", "border.png" );
    GLUtil::getInstance()->addTexture( "circle", "circle.png" );
    GLUtil::getInstance()->addTexture( "earth", "earth.png" );

    GLUtil::getInstance()->setCanvas( canvas );

    if ( headerSet )
        objectMan->setHeaderString( header );

    timer = new RenderTimer( canvas, timerInterval );
    //timer->Start();
    //wxStopWatch* t2 = new wxStopWatch();
    //videoSession_listener->setTimer( t2 );

    earth = new Earth();
    input = new InputHandler( objectMan, mainFrame );

    // frame needs reference to inputhandler to generate help window for
    // shortcut hotkeys
    mainFrame->setInputHandler( input );

    // add the input handler to the chain of things that can handle
    // events & give the canvas focus so we don't have to click on it to
    // start sending key events
    canvas->PushEventHandler( input );
    canvas->SetFocus();
    canvas->setTimer( timer );

    if ( !disablePython )
    {
        venueClientController = new VenueClientController( 0.0f, 0.0f,
                objectMan );
        venueClientController->setSessionControl( sessionTree );
    }

    objectMan->setEarth( earth );
    objectMan->setInput( input );
    objectMan->setTree( sourceTree );
    objectMan->setVideoListener( videoSessionListener );
    objectMan->setVenueClientController( venueClientController ); // may be null
    objectMan->setSessionManager( sessionManager );
    objectMan->setAudio( audioSessionListener ); // may not necessarily be used

    objectMan->setAutoFocusRotate( autoFocusRotate );
    objectMan->setGridAuto( gridAuto );

    if ( haveThumbnailFile )
    {
        std::string thumbPath = gravUtil::getInstance()->findFile(
                thumbnailFile );
        if ( thumbPath.compare( "" ) != 0 )
        {
            objectMan->setThumbnailMap(
                gravUtil::getInstance()->parseThumbnailFile( thumbPath ) );
        }
    }

    mapRTP();

    sessionTree->setSessionManager( sessionManager );
    sessionManager->setSessionTreeControl( sessionTree );
    sessionManager->setButtonTexture( "circle" );

    for ( unsigned int i = 0; i < initialVideoAddresses.size(); i++ )
    {
        gravUtil::logVerbose ( "grav::initializing video address %s\n",
                    initialVideoAddresses[i].c_str() );
        sessionTree->addSession( initialVideoAddresses[i], false,
                    addToAvailableVideoList );

        if ( haveVideoKey )
            sessionTree->setEncryptionKey( initialVideoAddresses[i],
                                            initialVideoKey );
    }
    for ( unsigned int i = 0; i < initialAudioAddresses.size(); i++ )
    {
        gravUtil::logVerbose ( "grav::initializing audio address %s\n",
                    initialAudioAddresses[i].c_str() );
        sessionTree->addSession( initialAudioAddresses[i], true, false );

        if ( haveAudioKey )
            sessionTree->setEncryptionKey( initialAudioAddresses[i],
                                            initialAudioKey );
    }

    if ( getAGVenueStreams && !disablePython )
    {
        venueClientController->updateVenueStreams();
        venueClientController->addAllVenueStreams();
    }

    sessionTree->setTimerInterval( rotateIntervalMS );
    if ( autoRotateAvailableVideo )
    {
        sessionTree->toggleAutomaticRotate();
        sessionTree->rotateVideoSessions( true );
    }

    gravUtil::logVerbose( "grav::init function complete\n" );
    return true;
}

int gravApp::OnExit()
{
    gravUtil::logVerbose( "grav::Exiting...\n" );
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

    delete earth;
    delete input;

    if ( venueClientController != NULL )
        delete venueClientController;
    delete objectMan;

    VPMPayloadDecoderFactory::shutdown();

    GLUtil::cleanupGL();
    PythonTools::cleanup();
    gravUtil::cleanup();

    return 0;
}

void gravApp::idleHandler( wxIdleEvent& evt )
{
    // start secondary thread if not running
    if ( usingThreads && !threadRunning )
    {
        objectMan->setThreads( usingThreads );
        threadRunning = true;
        VPMthread = thread_start( threadTest, this );
    }

    if ( !usingThreads )
        sessionManager->iterateSessions();

    if ( timerIntervalUS > 0 )
    {
        // this is the method for rendering on idle, with a limiter based on the
        // timer interval
        unsigned long time = (unsigned long)timer->getTiming();
        if ( time > (unsigned long)timerIntervalUS )
        {
            //gravUtil::logVerbose( "%lu\n", time );
            canvas->draw();
            timer->resetTiming();
        }
        else
        {
            wxMilliSleep( 1 );
        }
    }
    // otherwise (if fps value isn't set) just constantly draw - if vsync is on,
    // will be limited to vsync
    else if ( timerIntervalUS == 0 )
    {
        canvas->draw();
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
    }

    wxString headerWX;
    headerSet = parser.Found( _("header"), &headerWX );
    if ( headerSet )
    {
        header = std::string((char*)headerWX.char_str());
    }

    verbose = parser.Found( _("verbose") );
    VPMverbose = parser.Found( _("vpmedia-verbose") );

    printVersion = parser.Found( _("version") );

    usingThreads = !parser.Found( _("no-threads") );

    disablePython = parser.Found( _("no-python") );

    enableShaders = parser.Found( _("enable-shaders") );

    bufferFont = parser.Found( _("use-buffer-font") );

    startFullscreen = parser.Found( _("fullscreen") );

    addToAvailableVideoList = parser.Found( _("available-video-list") );

    getAGVenueStreams = parser.Found( _("get-ag-venue-streams") );

    autoFocusRotate = parser.Found( _("automatic") );

    gridAuto = parser.Found( _("gridauto") );

    fps = 0;
    if ( parser.Found( _("fps"), &fps ) )
    {
        timerInterval = floor( 1000.0f / (float)fps );
        timerIntervalUS = floor( 1000000.0f / (float)fps );
        // if it's not a valid fps value (>1000) reset to 60
        if ( timerInterval < 1 )
        {
            // log a message regarding this later - we can't use the normal wx
            // logging stuff at this point yet for some reason, causes weird
            // behavior
            timerInterval = 16;
            timerIntervalUS = 16667;
        }
    }
    // if fps isn't set, set these vals to 0 so the idle handler will just
    // constantly draw
    else
    {
        timerInterval = 0;
        timerIntervalUS = 0;
    }

    long int rotateIntervalS;
    autoRotateAvailableVideo = parser.Found( _("arav"), &rotateIntervalS );
    if ( autoRotateAvailableVideo )
    {
        // this is pretty dumb that the second value is a long int and the
        // millisecond interval is a regular int, but that's what the timer
        // input is for Start(), oh well
        rotateIntervalMS = (int)rotateIntervalS * 1000;
    }
    else
    {
        rotateIntervalMS = 30000;
    }

    wxString thumbnailFileWX;
    haveThumbnailFile = parser.Found( _("tf"), &thumbnailFileWX );
    if ( haveThumbnailFile )
    {
        thumbnailFile = std::string( thumbnailFileWX.char_str() );
    }

    wxString videoKeyWX;
    haveVideoKey = parser.Found( _("video-key"), &videoKeyWX );
    if ( haveVideoKey )
    {
        initialVideoKey = std::string( videoKeyWX.char_str() );
    }

    wxString audioKeyWX;
    haveAudioKey = parser.Found( _("audio-key"), &audioKeyWX );
    if ( haveAudioKey )
    {
        initialAudioKey = std::string( audioKeyWX.char_str() );
    }

    long int startXTemp, startYTemp, widthTemp, heightTemp;
    if ( parser.Found( _("start-x"), &startXTemp ) )
    {
        startX = startXTemp;
    }
    if ( parser.Found( _("start-y"), &startYTemp ) )
    {
        startY = startYTemp;
    }
    if ( parser.Found( _("start-width"), &widthTemp ) )
    {
        windowWidth = widthTemp;
    }
    if ( parser.Found( _("start-height"), &heightTemp ) )
    {
        windowHeight = heightTemp;
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
    gravUtil::logVerbose( "grav::starting network/decoding thread...\n" );
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
                gravUtil::logVerbose( "grav::thread still running\n" );
            threadCounter = (threadCounter+1)%1000;
        }
    }
    gravUtil::logVerbose( "grav::thread ending...\n" );
    return 0;
}

bool gravApp::isThreadRunning()
{
    return threadRunning;
}
