/*
 * @file SideFrame.h
 *
 * Definition of the GUI frame for the side window. Provides hooks to session
 * management in the menubar.
 *
 * Created on: Oct 12, 2010
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
