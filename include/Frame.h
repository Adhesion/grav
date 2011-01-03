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
    void OnCloseWindow( wxCloseEvent& evt );
    gravManager* grav;

    DECLARE_EVENT_TABLE()

};

#endif /* FRAME_H_ */
