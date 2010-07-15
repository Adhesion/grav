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

class Frame : public wxFrame
{

public:
    Frame( wxWindow* parent, wxWindowID id, const wxString& title,
                    const wxPoint& pos, const wxSize& size );

private:
    DECLARE_EVENT_TABLE()

    void OnCloseWindow( wxCloseEvent& evt );

};

#endif /* FRAME_H_ */
