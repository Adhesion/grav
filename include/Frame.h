/*
 * @file Frame.h
 *
 * Defines the GUI frame for the main video display window, as well as the side
 * list window.
 *
 * Created on: Jul 13, 2010
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

#ifndef FRAME_H_
#define FRAME_H_

#include <wx/wx.h>

class gravManager;
class InputHandler;

class Frame : public wxFrame
{

public:
    Frame( wxWindow* parent, wxWindowID id, const wxString& title );
    Frame( wxWindow* parent, wxWindowID id, const wxString& title,
                    const wxPoint& pos, const wxSize& size );
    void setSourceManager( gravManager* g );
    void setInputHandler( InputHandler* i );
    void spawnPropertyWindow( wxCommandEvent& evt );

private:
    // different event types for these, so we need these separate functions even
    // though they do the same thing
    void OnCloseWindow( wxCloseEvent& evt );
    void OnExit( wxCommandEvent& evt );

    void OnAbout( wxCommandEvent& evt );
    void OnKeyboardShortcuts( wxCommandEvent& evt );

    void OnMenuOpen( wxMenuEvent& evt );

    void setupMenuBar();

    // common close functionality to be accessed by the different entry points
    void cleanup();

    gravManager* grav;
    // reference to input to grab keyboard shortcuts to generate help menu
    InputHandler* input;

    void toggleRunwayEvent( wxCommandEvent& evt );
    void toggleVCCEvent( wxCommandEvent& evt );
    void toggleAutomaticEvent( wxCommandEvent& evt );

    // IDs for toggles in view section of menubar
    static int toggleRunwayID;
    static int toggleVCCID;
    static int toggleAutomaticID;

    DECLARE_EVENT_TABLE()

};

#endif /* FRAME_H_ */
