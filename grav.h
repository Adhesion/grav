#ifndef GRAV_H_
#define GRAV_H_

/**
 * @file grav.h
 *
 * Header file for main grav app - contains the definition for the main class,
 * which acts as the main WX app/controller and OnInit() which acts as the
 * main().
 *
 * @author Andrew Ford
 */

#include <wx/wx.h>
#include <wx/cmdline.h>
#include <VPMedia/thread_helper.h>

class GLCanvas;
class Timer;
class Frame;
class gravManager;
class TreeControl;
class VideoListener;
class AudioManager;
class VPMSessionFactory;
class VPMSession;

class gravApp : public wxApp
{

private:

    /**
     * Init function for WX - acts as the main
     */
    virtual bool OnInit();
    virtual int OnExit();
    
    DECLARE_EVENT_TABLE()
    
    void idleHandler( wxIdleEvent& evt );

    /*
     * Create a new RTP session and attach it to the proper listener. If
     * audio is true it's an audio session; video if false.
     * Returns false if session creation fails, true otherwise.
     */
    bool initSession( std::string address, bool audio );

    void iterateSessions();

    /**
     * Parse the command line arguments and set options accordingly.
     * Primarily for setting the video/audio/etc addresses.
     */
    bool handleArgs();
    
    /**
     * Map RTP payloads that aren't determined statically by the RFC
     * (those that are have already been mapped in VPMedia)
     */
    void mapRTP();
    
    static void* threadTest( void* args );
    bool isThreadRunning();

    wxCmdLineParser parser;
    
    Frame* mainFrame;
    Frame* treeFrame;
    
    GLCanvas* canvas;
    Timer* timer;
    TreeControl* tree;
    
    int timerInterval;
    int timerIntervalUS;

    gravManager* grav;
    
    bool usingThreads;
    bool threadRunning;
    thread* VPMthread;

    VPMSessionFactory *sf;

    VPMSession* videoSession;
    uint32_t videoSession_ts;
    VideoListener* videoSession_listener;
    bool videoInitialized;

    bool audioEnabled;
    VPMSession* audioSession;
    uint32_t audioSession_ts;
    AudioManager* audioSession_listener;
    bool audioInitialized;

    bool disableShaders;

    int windowWidth, windowHeight;
    
};

static const wxCmdLineEntryDesc cmdLineDesc[] =
{
    { wxCMD_LINE_SWITCH, _("h"), _("help"), _("displays the help message"),
        wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },

    { wxCMD_LINE_SWITCH, _("t"), _("threads"), _("enables threading separation"
            " of graphics and network/decoding") },

    { wxCMD_LINE_SWITCH, _("am"), _("automatic"), _("automatically tiles"
            " objects when added - if not, uses runway") },

    { wxCMD_LINE_SWITCH, _("ds"), _("disable-shaders"), _("disable GLSL shader-"
            "based colorspace conversion if it would be available") },

    { wxCMD_LINE_OPTION, _("a"), _("audio"), _("RTP audio session address"),
        wxCMD_LINE_VAL_STRING },

    { wxCMD_LINE_OPTION, _("f"), _("fps"), _("framerate for rendering"),
        wxCMD_LINE_VAL_NUMBER },

    { wxCMD_LINE_PARAM, NULL, NULL, _("video address"), wxCMD_LINE_VAL_STRING },

    { wxCMD_LINE_NONE }
};

#endif /*GRAV_H_*/
