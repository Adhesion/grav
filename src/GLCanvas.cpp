/*
 * @file GLCanvas.h
 *
 * Implementation of the GL canvas class. Also defines a timer for defining
 * when to render.
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

#include "ObjectManager.h"
#include "gravUtil.h"
#include "GLCanvas.h"
#include "InputHandler.h"
#include "Timers.h"

BEGIN_EVENT_TABLE(GLCanvas, wxGLCanvas)
EVT_PAINT(GLCanvas::handlePaintEvent)
EVT_SIZE(GLCanvas::resize)
END_EVENT_TABLE()

GLCanvas::GLCanvas( wxWindow* parent, ObjectManager* o, int* attributes,
                        wxSize size ) :
    wxGLCanvas( parent, wxID_ANY, attributes, wxDefaultPosition,
                wxDefaultSize, 0, wxT("grav GLCanvas")), objectMan( o )
{
    SetSize( size );
    glContext = new wxGLContext( this );
    SetCurrent( *glContext );

    lastDrawTime = 0;
    lastNonDrawTime = 0;
    lastDrawTimeMax = 0;
    lastNonDrawTimeMax = 0;
    counter = 0;
    counterMax = 5;

    fpsCounter = 0;
    fpsResult = 0;

    useDebugTimers = false;
    renderTimer = NULL;
}

GLCanvas::~GLCanvas()
{
    delete glContext;
    stopTimer();
}

void GLCanvas::handlePaintEvent( wxPaintEvent& evt )
{
    draw();
}

void GLCanvas::draw()
{
    if ( useDebugTimers )
    {
        lastNonDrawTime = drawStopwatch.Time();
        lastNonDrawTimeMax += lastNonDrawTime;
        drawStopwatch.Start();
    }

    if( !IsShown() ) return;

    SetCurrent( *glContext );
    wxPaintDC( this );

    if ( objectMan != NULL )
        objectMan->draw();

    SwapBuffers();

    if ( useDebugTimers )
    {
        lastDrawTime = drawStopwatch.Time();
        lastDrawTimeMax += lastDrawTime;
        drawStopwatch.Start();

        counter = (counter+1) % counterMax;
        if ( counter == 0 )
        {
            lastDrawTimeAvg = lastDrawTimeMax / (float)counterMax;
            lastNonDrawTimeAvg = lastNonDrawTimeMax / (float)counterMax;
            lastDrawTimeMax = 0;
            lastNonDrawTimeMax = 0;
        }

        fpsCounter++;
        long elapsed = fpsStopwatch.Time();
        if ( elapsed > 500 )
        {
            fpsResult = (float)fpsCounter * 1000.0f / (float)elapsed;
            fpsCounter = 0;
            fpsStopwatch.Start();
        }
    }
}

void GLCanvas::resize( wxSizeEvent& evt )
{
    gravUtil::logVerbose( "GLCanvas::resize: resize callback: to %ix%i\n",
            evt.GetSize().GetWidth(), evt.GetSize().GetHeight() );
    OnSize( evt );
    Refresh( false );
    GLreshape( evt.GetSize().GetWidth(), evt.GetSize().GetHeight() );
}

void GLCanvas::GLreshape( int w, int h )
{
    glViewport(0, 0, w, h);

    if (w > h)
    {
        screen_height = 1.0;
        screen_width = (float)w/(float)h;
    }
    else
    {
        screen_height = (float)h/(float)w;
        screen_width = 1.0;
    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-screen_width/10.0, screen_width/10.0,
              -screen_height/10.0, screen_height/10.0,
              0.1, 50.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(objectMan->getCamX(), objectMan->getCamY(), objectMan->getCamZ(),
              0.0, 0.0, -25.0,
              0.0, 1.0, 0.0);

    // note this should be done last since stuff inside setwindowsize
    // (finding the world space bounds for the screen) depends on the matrices
    // above being accurate
    objectMan->setWindowSize( w, h );
}

void GLCanvas::stopTimer()
{
    if ( renderTimer != NULL )
    {
        renderTimer->Stop();
    }
}

void GLCanvas::setTimer( RenderTimer* t )
{
    if ( renderTimer != NULL )
        renderTimer = t;
}

long GLCanvas::getDrawTime()
{
    return lastDrawTimeAvg;
}

long GLCanvas::getNonDrawTime()
{
    return lastNonDrawTimeAvg;
}

long GLCanvas::getDrawTimeAvg()
{
    return lastDrawTimeAvg;
}

long GLCanvas::getNonDrawTimeAvg()
{
    return lastNonDrawTimeAvg;
}

float GLCanvas::getFPS()
{
    return fpsResult;
}

void GLCanvas::setDebugTimerUsage( bool d )
{
    useDebugTimers = d;
    if ( d )
        fpsStopwatch.Start();
    else
    {
        fpsStopwatch.Pause();
        fpsCounter = 0;
    }
}

bool GLCanvas::getDebugTimerUsage()
{
    return useDebugTimers;
}
