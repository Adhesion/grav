/**
 * SideFrame.h
 *
 * Definition of the GUI frame for the side window. Provides hooks to session
 * management in the menubar.
 *
 * Created on: Oct 12, 2010
 * @author Andrew Ford
 */

#ifndef SIDEFRAME_H_
#define SIDEFRAME_H_

#include <wx/wx.h>

class SessionTreeControl;

class SideFrame : public wxFrame
{

public:
    SideFrame( wxWindow* parent, wxWindowID id, const wxString& title );
    SideFrame( wxWindow* parent, wxWindowID id, const wxString& title,
                    const wxPoint& pos, const wxSize& size );

    void addVideoSessionEvent( wxCommandEvent& evt );
    void addAudioSessionEvent( wxCommandEvent& evt );
    void rotateEvent( wxCommandEvent& evt );

    static int addVideoID;
    static int addAudioID;
    static int rotateID;

private:
    SessionTreeControl* findSessionTree( wxWindow* w );

    void setupMenuBar();

};

#endif /* SIDEFRAME_H_ */
