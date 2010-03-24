#ifndef GRAV_H_
#define GRAV_H_

/**
 * @file grav.h
 * Header file for main grav app - contains the definition for the main class,
 * which acts as the main WX app/controller and OnInit() which acts as the
 * main().
 * @author Andrew Ford
 */

#include <wx/wx.h>
#include <wx/cmdline.h>
#include <VPMedia/thread_helper.h>

class GLCanvas;
class gravManager;
class TreeControl;

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
    void iterate();

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

    wxCmdLineParser parser;
    
    wxFrame* mainFrame;
    wxFrame* treeFrame;
    
    GLCanvas* canvas;
    TreeControl* tree;
    
    gravManager* grav;
    
    bool usingThreads;
    thread* VPMthread;

    int windowWidth, windowHeight;
    
};

static const wxCmdLineEntryDesc cmdLineDesc[] =
{
    { wxCMD_LINE_SWITCH, _("h"), _("help"), _("displays the help message"),
        wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
    
    { wxCMD_LINE_SWITCH, _("t"), _("threads"), _("enables threading separation"
            " of graphics and network/decoding") },

    { wxCMD_LINE_OPTION, _("a"), _("audio"), _("RTP audio session address"),
        wxCMD_LINE_VAL_STRING },
    
    { wxCMD_LINE_PARAM, NULL, NULL, _("video address"), wxCMD_LINE_VAL_STRING },
    
    { wxCMD_LINE_NONE }
};

#endif /*GRAV_H_*/
