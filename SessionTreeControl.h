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

class SessionTreeControl : public wxTreeCtrl
{

public:
    SessionTreeControl();
    SessionTreeControl( wxWindow* parent );
    void addSession( std::string address, bool audio );

private:
    wxTreeItemId rootID;
    wxTreeItemId videoNodeID;
    wxTreeItemId audioNodeID;

};

#endif /* SESSIONTREECONTROL_H_ */
