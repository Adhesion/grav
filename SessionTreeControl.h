/**
 * @file SessionTreeControl.h
 *
 * Definition of the SessionTreeControl, which is a tree structure for storing
 * audio & video sessions as displayed in the side window.
 *
 * Created on: Sep 13, 2010
 * @author Andrew Ford
 */

#ifndef SESSIONTREECONTROL_H_
#define SESSIONTREECONTROL_H_

#include <wx/treectrl.h>

class SessionManager;

class SessionTreeControl : public wxTreeCtrl
{

public:
    SessionTreeControl();
    SessionTreeControl( wxWindow* parent );
    void setSessionManager( SessionManager* s );

    void addSession( std::string address, bool audio );
    void removeSession( std::string address );
    wxTreeItemId findSession( wxTreeItemId root, std::string address );

    void itemRightClick( wxTreeEvent& evt );
    void toggleEnableEvent( wxCommandEvent& evt );
    void removeEvent( wxCommandEvent& evt );

    static int toggleEnableID;
    static int removeID;

private:
    wxTreeItemId rootID;
    wxTreeItemId videoNodeID;
    wxTreeItemId audioNodeID;

    SessionManager* sessionManager;

    DECLARE_EVENT_TABLE()

};

#endif /* SESSIONTREECONTROL_H_ */
