/*
 * @file VenueClientController.h
 *
 * Class that uses PythonTools and AG-based python scripts for controlling
 * AG Venue Clients (moving between venues, etc.) and provides grav interface
 * elements for access.
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

#ifndef VENUECLIENTCONTROLLER_H_
#define VENUECLIENTCONTROLLER_H_

#include <string>
#include <map>

#include "PythonTools.h"
#include "Group.h"

class gravManager;
class SessionTreeControl;

class VenueClientController : public Group
{

public:
    VenueClientController( float _x, float _y, gravManager* g );
    ~VenueClientController();

    void draw();

    /*
     * These remove variants remove the member objects entirely (including
     * remove from main lists and delete) since this object effectively owns
     * them.
     */
    void remove( RectangleBase* object, bool move = true );
    std::vector<RectangleBase*>::iterator remove(
                    std::vector<RectangleBase*>::iterator i, bool move = true );

    /*
     * Attempt to find the first valid venue client via AGTools. If it fails,
     * return false and set internal venue client URL to "".
     * Will automatically hide if there is no venue client found. The bool
     * argument determines animation for this.
     */
    bool tryGetValidVenueClient( bool instantHide = false );

    void updateExitMap();
    void printExitMap();

    void enterVenue( std::string venueName );
    void updateVenueStreams();
    void updateVenueName();
    void removeAllVenueStreams();
    void addAllVenueStreams();

    void rearrange();

    bool updateName();
    void show( bool s, bool instant = false );

    void setSessionControl( SessionTreeControl* s );

private:
    std::map<std::string, std::string> exitMap;
    std::string currentVenue;
    // map of addresses to encryption keys
    std::map<std::string, std::string> currentVenueStreams;
    std::string venueClientUrl;

    // note we don't own any of these pointers, but we do own the VenueNode
    // children which is why we delete them in (inherited, overridden) remove()
    PythonTools* pyTools;
    std::string AGToolsScript;

    gravManager* grav;

    SessionTreeControl* sessionControl;

};

#endif /* VENUECLIENTCONTROLLER_H_ */
