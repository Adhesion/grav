/*
 * @file VenueClientController.cpp
 *
 * Implementation of a class for controlling Access Grid Venue Clients.
 *
 * Created on: Nov 8, 2010
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

#include "VenueClientController.h"
#include "gravManager.h"
#include "SessionTreeControl.h"
#include "VenueNode.h"
#include "PNGLoader.h"
#include "gravUtil.h"

VenueClientController::VenueClientController( float _x, float _y,
                                                gravManager* g )
    : Group( _x, _y ), grav( g )
{
    locked = false;
    selectable = false;
    userMovable = false;
    setName( "Venues" );
    debugDraw = false;

    baseBColor.R = 0.7f;
    baseBColor.G = 0.7f;
    baseBColor.B = 1.0f;
    baseBColor.A = 0.8f;
    destBColor = baseBColor;
    borderColor = destBColor;

    setScale( 13.0f, 13.0f );

    gravUtil* util = gravUtil::getInstance();
    pyTools = PythonTools::getInstance();

    // this should be safe since this constructor normally gets called after
    // GL stuff gets set up
    circleWidth = 256;
    circleHeight = 256;
    circleTex = 0;
    std::string circleLoc = util->findFile( "circle.png" );
    if ( circleLoc.compare( "" ) == 0 )
    {
        gravUtil::logWarning( "VenueClientController::init: warning: "
                "texture circle.png not found\n" );
    }
    else
    {
        circleTex = PNGLoader::loadPNG( circleLoc, circleWidth, circleHeight );
    }

    AGToolsScript = util->findFile( "AGTools.py" );
    if ( AGToolsScript.compare( "" ) == 0 )
    {
        gravUtil::logWarning( "VenueClientController::warning: "
                "AGTools.py not found\n" );
    }

    // if we have a valid venue client, this show call will update
    // the venue name and exits as well as hide the interface by default
    show( false, true );
}

VenueClientController::~VenueClientController()
{
    removeAll();
}

void VenueClientController::draw()
{
    animateValues();

    if ( borderColor.A < 0.01f )
        return;

    glColor4f( borderColor.R, borderColor.G, borderColor.B, borderColor.A );

    // draw lines from center to each of the venues
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glLineWidth( 2.0f );
    glEnable( GL_LINE_SMOOTH );
    glBegin( GL_LINES );
    for ( unsigned int i = 0; i < objects.size(); i++ )
    {
        glVertex3f( objects[i]->getX(), objects[i]->getY(), 0.0f );
        glVertex3f( getX(), getY(), 0.0f );
    }
    glEnd();
    glDisable( GL_LINE_SMOOTH );
    glDisable( GL_BLEND );
    glLineWidth( 1.0f );

    for ( unsigned int i = 0; i < objects.size(); i++ )
    {
        objects[i]->draw();
    }
}

void VenueClientController::remove( RectangleBase* object, bool move )
{
    Group::remove( object, move );
    grav->removeFromLists( object, false );
    delete object;
}

std::vector<RectangleBase*>::iterator VenueClientController::remove(
                            std::vector<RectangleBase*>::iterator i, bool move )
{
    RectangleBase* object = (*i);
    std::vector<RectangleBase*>::iterator ret = Group::remove( i, move );
    grav->removeFromLists( object, false );
    delete object;
    return ret;
}

bool VenueClientController::tryGetValidVenueClient( bool instantHide )
{
    PyObject* res = pyTools->call( AGToolsScript, "GetFirstValidClientURL" );
    if ( res != NULL && res != Py_None )
    {
        venueClientUrl = PyString_AsString( res );
    }
    else
    {
        venueClientUrl = "";
    }
    Py_XDECREF( res );

    if ( venueClientUrl.compare( "" ) == 0 )
    {
        gravUtil::logWarning( "VenueClientController::tryGetValidVenueClient: "
                "no venue clients found\n" );
        /*
         * Since this function is equivalent to "is the VCC showable", if it's
         * not showable, forcibly hide it if it is shown, since that state makes
         * no sense if there is no available venue client.
         */
        if ( shown )
        {
            Group::show( false, instantHide );
            // move objects for a nice animation effect
            for ( unsigned int i = 0; i < objects.size(); i++ )
            {
                objects[i]->move( getX(), getY() );
            }
        }
        return false;
    }
    else
    {
        return true;
    }
}

void VenueClientController::updateExitMap()
{
    PyObject* pRes = pyTools->call( AGToolsScript, "GetExits",
                                    venueClientUrl );
    exitMap = pyTools->dtom( pRes );
    Py_XDECREF( pRes );

    // TODO check if exitMap changes here, to avoid needless remake?
    removeAll();
    std::map<std::string, std::string>::iterator i;
    for ( i = exitMap.begin(); i != exitMap.end(); ++i )
    {
        VenueNode* node = new VenueNode();
        node->setName( i->first );
        node->setTexture( circleTex, circleWidth, circleHeight );
        grav->addToDrawList( node );
        add( node );
    }
}

void VenueClientController::printExitMap()
{
    //updateExitMap();

    if ( venueClientUrl.compare( "" ) == 0 )
    {
        return;
    }
    gravUtil::logVerbose( "VenueClientController::printExitMap: "
            "from venue client at %s\n", venueClientUrl.c_str() );
    std::map<std::string, std::string>::iterator it;
    for ( it = exitMap.begin(); it != exitMap.end(); ++it )
    {
        gravUtil::logVerbose( "\t%s : %s\n", it->first.c_str(),
                it->second.c_str() );
    }

    gravUtil::logVerbose( "\tObjects, should correspond to venues:\n" );
    for ( unsigned int i = 0; i < objects.size(); i++ )
    {
        gravUtil::logVerbose( "\t%i: %s\n", i,
                objects[i]->getSubName().c_str() );
    }
}

void VenueClientController::enterVenue( std::string venueName )
{
    std::map<std::string, std::string>::iterator it = exitMap.find( venueName );
    if ( it == exitMap.end() )
    {
        gravUtil::logWarning( "VenueClientController::enterVenue: "
                "venue %s not found\n", venueName.c_str() );
        return;
    }

    removeAllVenueStreams();

    PyObject* args = PyTuple_New( 2 );
    PyTuple_SetItem( args, 0, PyString_FromString( venueClientUrl.c_str() ) );
    PyTuple_SetItem( args, 1, PyString_FromString( it->second.c_str() ) );

    gravUtil::logVerbose( "VenueClientController::calling entervenue on %s to"
            " %s\n", venueClientUrl.c_str(), it->second.c_str() );

    pyTools->call( AGToolsScript, "EnterVenue", args );

    // this will in turn update the exit map, venue streams, etc.
    show( false );

    addAllVenueStreams();
}

void VenueClientController::updateVenueStreams()
{
    std::string type = "video";
    PyObject* args = PyTuple_New( 2 );
    PyTuple_SetItem( args, 0, PyString_FromString( venueClientUrl.c_str() ) );
    PyTuple_SetItem( args, 1, PyString_FromString( type.c_str() ) );

    PyObject* res = pyTools->call( AGToolsScript, "GetFormattedVenueStreams",
                                    args );
    currentVenueStreams = pyTools->dtom( res );
    Py_XDECREF( res );
}

void VenueClientController::updateVenueName()
{
    PyObject* res = pyTools->call( AGToolsScript, "GetCurrentVenueName",
                                    venueClientUrl.c_str() );
    if ( res != NULL && res != Py_None )
        currentVenue = PyString_AsString( res );
    else
        currentVenue = "";
    Py_XDECREF( res );
}

void VenueClientController::removeAllVenueStreams()
{
    std::map<std::string, std::string>::iterator it;
    for ( it = currentVenueStreams.begin(); it != currentVenueStreams.end();
            ++it )
    {
        gravUtil::logVerbose( "VenueClientController::remove(): "
                "Video stream: %s\n", it->first.c_str() );
        sessionControl->removeSession( it->first );
    }
}

void VenueClientController::addAllVenueStreams()
{
    std::map<std::string, std::string>::iterator it;
    for ( it = currentVenueStreams.begin(); it != currentVenueStreams.end();
            ++it )
    {
        gravUtil::logVerbose( "VenueClientController::add(): "
                "Video stream: %s\n", it->first.c_str() );
        sessionControl->addSession( it->first, false, false );
        // __NO_KEY__ is a dummy value to indicate there is no encryption on the
        // stream in question
        if ( it->second.compare( "__NO_KEY__" ) != 0 )
        {
            sessionControl->setEncryptionKey( it->first, it->second );
        }
    }
}

void VenueClientController::rearrange()
{
    std::map<std::string, std::string> opts;

    std::map<std::string, std::vector<RectangleBase*> > data;
    data["objects"] = objects;

    RectangleBase smaller = *this;
    // uneven since most screens will be widescreen - so make vertical area
    // bigger so objects on top/bottom are bigger and text more readable
    smaller.setScale( smaller.getScaleX() * 0.6f,
                        smaller.getScaleY() * 0.45f );
    layouts.arrange( "perimeter", *this, smaller, data, opts );
}

bool VenueClientController::updateName()
{
    // like runway, this does nothing
    return false;
}

void VenueClientController::show( bool s, bool instant )
{
    if ( tryGetValidVenueClient( instant ) )
    {
        // check if venue has changed in the meantime, if so update stuff
        std::string oldName = currentVenue;
        updateVenueName();

        if ( oldName.compare( currentVenue ) != 0 )
        {
            updateExitMap();
            updateVenueStreams();
        }
    }
    else
    {
        return;
        // tryGetValidVenueClient() will hide the object itself if it is shown &
        // the venue client check fails
    }

    // do nothing if there aren't any venues, otherwise state will get confusing
    // to the user (ie, shown with no exits, then venue move in AG, next ctrl-v
    // hit will "hide" nothing, opposite what is expected)
    if ( exitMap.size() == 0 )
    {
        return;
    }

    Group::show( s, instant );
    if ( !s )
    {
        // move objects for a nice animation effect
        for ( unsigned int i = 0; i < objects.size(); i++ )
        {
            objects[i]->move( getX(), getY() );
        }
    }
    else
    {
        rearrange();
        grav->moveToTop( this );
    }
}

void VenueClientController::setSessionControl( SessionTreeControl* s )
{
    sessionControl = s;
}
