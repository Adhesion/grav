/*
 * @file GLCanvas.h
 *
 * Declaration of the GL canvas class, which defines the main rendered area
 * and draws it.
 *
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

#ifndef GLCANVAS_H_
#define GLCANVAS_H_

#include "GLUtil.h"

// note GLUtil needs to be included first, since glew/glxew needs to set up its
// #defines and such before other libs like glu are included (via wxglcanvas)
#include <wx/glcanvas.h>
#include <wx/stopwatch.h>

#include <sys/time.h>

class gravManager;
class RenderTimer;

class GLCanvas : public wxGLCanvas
{

public:
    GLCanvas( wxWindow* parent, gravManager* g, int* attributes, wxSize size );
    ~GLCanvas();

    void handlePaintEvent( wxPaintEvent& evt );
    void draw();
    void resize( wxSizeEvent& evt );
    void GLreshape( int w, int h );

    void stopTimer();
    void setTimer( RenderTimer* t );

    long getDrawTime();
    long getNonDrawTime();
    long getDrawTimeAvg();
    long getNonDrawTimeAvg();
    float getFPS();

    void setDebugTimerUsage( bool d );
    bool getDebugTimerUsage();

private:
    gravManager* grav;
    wxGLContext* glContext;
    DECLARE_EVENT_TABLE()

    void testDraw();
    void testKey( wxKeyEvent& evt );

    // tracks the aspect ratio of the screen for reshaping
    float screen_width, screen_height;

    // if draw is being called by a timer, have a reference to it so we can stop
    // it if need be
    RenderTimer* renderTimer;

    // for measuring the draw time
    wxStopWatch drawStopwatch;
    long lastDrawTime;
    long lastNonDrawTime;

    // for calculating averages
    long lastDrawTimeMax;
    long lastNonDrawTimeMax;
    long lastDrawTimeAvg;
    long lastNonDrawTimeAvg;
    int counter;
    int counterMax;

    wxStopWatch fpsStopwatch;
    int fpsCounter;
    float fpsResult;

    bool useDebugTimers;

};

#endif /*GLCANVAS_H_*/
