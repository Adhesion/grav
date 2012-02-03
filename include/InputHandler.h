/*
 * @file InputHandler.h
 *
 * Processes keyboard and mouse input (passed from GLUT) and controls object
 * selection and movement accordingly.
 *
 * @author Andrew Ford
 * @author Ralph Bean
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

#ifndef INPUTHANDLER_H_
#define INPUTHANDLER_H_

#include <vector>
#include <map>
#include <string>

#include <wx/wx.h>

#include "LayoutManager.h"

class VideoSource;
class RectangleBase;
class Earth;
class Group;
class ObjectManager;
class LayoutManager;
class Frame;

typedef double GLdouble;

class InputHandler;
typedef void (InputHandler::*MFP)(void);

class InputHandler : public wxEvtHandler
{

public:
    InputHandler( Earth* e, ObjectManager* g, Frame* f );
    ~InputHandler();

    void wxKeyDown( wxKeyEvent& evt );
    void wxCharEvt( wxKeyEvent& evt );
    void wxMouseMove( wxMouseEvent& evt );
    void wxMouseLDown( wxMouseEvent& evt );
    void wxMouseLUp( wxMouseEvent& evt );
    void wxMouseLDClick( wxMouseEvent& evt );
    void wxMouseRDown( wxMouseEvent& evt );

    void handlePrintSelected();
    void handleRearrangeGroups();
    void handleUpdateGroupNames();
    void handlePerimeterArrange();
    void handleGridArrange();
    void handleFocusArrange();
    void handleFullscreenSelectedSingle();
    void handleFullerFullscreenSelectedSingle();
    void handleRunwayToggle();
    void handleInvertSelection();
    void handleInformation();
    void handleToggleGroupLocks();
    void handleMuteSelected();
    void handleRandomTest();
    void handleNativeScaleAll();
    void handleNativeScaleSelected();
    void handleMoveAllToCenter();
    void handleToggleSiteGrouping();
    void handleToggleShowVenueClientController();
    void handleToggleRenderingSelected();
    void handleZoomout();
    void handleZoomin();
    void handleStrafeLeft();
    void handleStrafeRight();
    void handleCameraReset();
    void handleToggleAutoFocusRotate();
    void handleSelectAll();
    void handleToggleGraphicsDebug();
    void handleDownscaleSelected();
    void handleUpscaleSelected();
    void handleToggleFullscreen();
    void handleQuit();
    void handleClearSelected();
    void handleAddTestObject();
    void handleTryDeleteObject();
    void handleHelp();

    void processKeyboard( int keyCode, int x, int y );

    void leftClick( bool doubleClick = false );
    void leftRelease();
    void mouseLeftHeldMove();

    bool selectVideos();
    static int propertyID;

    /*
     * Various accessors. Note that the positions are in world space (ie not
     * screen pixels).
     */
    bool isLeftButtonHeld();
    float getMouseX(); float getMouseY();
    float getDragStartX(); float getDragStartY();
    float getDragEndX(); float getDragEndY();
    bool haveValidMousePos();

    /*
     * From the docstr dict, generate a list of strings that acts as a help file
     * for keyboard shortcuts.
     */
    std::map<std::string, std::string> getShortcutHelpList();

private:
    std::vector<RectangleBase*>* tempSelectedObjects;
    Earth* earth;

    // parent class
    ObjectManager* objectMan;

    // main gui window so we can trigger the proper quit sequence
    Frame* mainFrame;

    LayoutManager layouts;

    std::map<char, std::string> unprintables;
    std::map<int, MFP> lookup;
    std::map<int, std::string> docstr;
    int ktoh( unsigned char key ); // key to hash
    int ktoh( unsigned char key, int modifiers ); // key to hash
    unsigned char htok( int keyHash ); // hash to key
    std::string htos( int keyHash ); // Hash to string representation

    GLdouble mouseX, mouseY;

    // start & end pos for click-and-dragging
    float dragStartX;
    float dragStartY;
    float dragEndX;
    float dragEndY;
    // the mouse pos from a previous activemotion call, for calculating drag
    // distance
    float dragPrevX;
    float dragPrevY;

    bool leftButtonHeld;

    // if the camera is in a weird state (shouldn't happen normally, but just in
    // case), the camera lookat may not intersect with the screen rectangle
    // (the plane all the videos etc. are on), so the mouse position will be
    // undefined. (actually the posX/posY will just hold the previous value)
    // this will signify if we're in that state.
    // note this may change later, eg if it's ever possible to click on other
    // 3d objects
    bool validMousePos;

    // this should only be for mouse callbacks
    bool ctrlHeld;
    bool clickedInside;
    bool dragging;

    // use wx modifiers for keyboard callback - to easily check for "shift only"
    // "ctrl only" etc.
    int modifiers;

    DECLARE_EVENT_TABLE()

};

#endif /*INPUTHANDLER_H_*/
