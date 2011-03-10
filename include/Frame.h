/**
 * @file Frame.h
 *
 * Defines the GUI frame for the main video display window, as well as the side
 * list window.
 *
 * Created on: Jul 13, 2010
 * @author Andrew Ford
 */

#ifndef FRAME_H_
#define FRAME_H_

#include <wx/wx.h>

class gravManager;

class Frame : public wxFrame
{

public:
    Frame( wxWindow* parent, wxWindowID id, const wxString& title );
    Frame( wxWindow* parent, wxWindowID id, const wxString& title,
                    const wxPoint& pos, const wxSize& size );
    void setSourceManager( gravManager* g );
    void spawnPropertyWindow( wxCommandEvent& evt );

private:
    // different event types for these, so we need these separate functions even
    // though they do the same thing
    void OnCloseWindow( wxCloseEvent& evt );
    void OnExit( wxCommandEvent& evt );

    void OnAbout( wxCommandEvent& evt );

    void setupMenuBar();

    // common close functionality to be accessed by the different entry points
    void cleanup();

    gravManager* grav;

    DECLARE_EVENT_TABLE()

};

#endif /* FRAME_H_ */
