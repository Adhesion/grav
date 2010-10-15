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
#include <wx/notebook.h>
#include <VPMedia/thread_helper.h>

#include <vector>

class GLCanvas;
class RenderTimer;
class RotateTimer;
class Frame;
class SideFrame;
class gravManager;
class TreeControl;
class SessionTreeControl;
class VideoListener;
class AudioManager;
class SessionManager;

class gravApp : public wxApp
{

public:
    static bool threadDebug;
    static int threadCounter;

private:

    /**
     * Init function for WX - acts as the main
     */
    virtual bool OnInit();
    virtual int OnExit();
    
    DECLARE_EVENT_TABLE()
    
    void idleHandler( wxIdleEvent& evt );

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
    /**
     * Calls iterate sessions on the session manager, but also sleeps if there
     * was no iteration (to avoid CPU spin). Used when sessions are on separate
     * thread.
     */
    void iterateSessions();

    wxCmdLineParser parser;
    
    Frame* mainFrame;
    SideFrame* treeFrame;
    wxPanel* treePanel;
    wxNotebook* treeNotebook;
    
    GLCanvas* canvas;
    RenderTimer* timer;
    TreeControl* sourceTree;
    SessionTreeControl* sessionTree;
    
    int timerInterval;
    int timerIntervalUS;

    bool autoVideoSessionRotate;
    int rotateIntervalMS;
    RotateTimer* rotateTimer;

    gravManager* grav;

    bool usingThreads;
    bool threadRunning;
    thread* VPMthread;

    std::vector<std::string> initialVideoAddresses;
    //std::vector<VPMSession*> videoSessions;
    //uint32_t videoSession_ts;
    VideoListener* videoSessionListener;
    //bool videoInitialized;

    std::vector<std::string> initialAudioAddresses;
    //bool audioEnabled;
    //std::vector<VPMSession*> audioSessions;
    //uint32_t audioSession_ts;
    AudioManager* audioSessionListener;
    //bool audioInitialized;

    SessionManager* sessionManager;

    std::string header;
    bool headerSet;

    bool enableShaders;

    bool startFullscreen;

    bool videoSessionRotate;

    int windowWidth, windowHeight;

};

static const wxCmdLineEntryDesc cmdLineDesc[] =
{
    {
        wxCMD_LINE_SWITCH, _("h"), _("help"), _("displays the help message"),
            wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP
    },

    {
        wxCMD_LINE_SWITCH, _("t"), _("threads"),
            _("enables threading separation of graphics and network/decoding")
    },

    {
        wxCMD_LINE_SWITCH, _("am"), _("automatic"),
            _("automatically tiles objects when added - if not, uses runway")
    },

    {
        wxCMD_LINE_SWITCH, _("es"), _("enable-shaders"),
            _("enable GLSL shader-based colorspace conversion if it would be "
                "available (experimental, may not look as good, adds CPU usage "
                "to rendering thread)")
    },

    {
        wxCMD_LINE_OPTION, _("a"), _("audio"), _("RTP audio session address"),
            wxCMD_LINE_VAL_STRING
    },

    {
        wxCMD_LINE_OPTION, _("ht"), _("header"), _("Header string"),
            wxCMD_LINE_VAL_STRING
    },

    {
        wxCMD_LINE_OPTION, _("fps"), _("framerate"),
            _("framerate for rendering"), wxCMD_LINE_VAL_NUMBER
    },

    {
        wxCMD_LINE_SWITCH, _("fs"), _("fullscreen"),
            _("start in fullscreen mode")
    },

    {
        wxCMD_LINE_SWITCH, _("ga"), _("gridauto"),
            _("rearrange objects in grid on source add/remove")
    },

    {
        wxCMD_LINE_SWITCH, _("vsr"), _("video-session-rotate"),
            _("periodically rotate through supplied video addresses")
    },

    {
        wxCMD_LINE_OPTION, _("avsr"), _("auto-video-session-rotate"),
            _("rotate video sessions every [num] seconds"),
            wxCMD_LINE_VAL_NUMBER
    },

    {
        wxCMD_LINE_PARAM, NULL, NULL, _("video address"),
            wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_MULTIPLE
    },

    {
        wxCMD_LINE_NONE
    }
};

#endif /*GRAV_H_*/
