/*
 * VenueClientController.h
 *
 * Class that uses PythonTools and AG-based python scripts for controlling
 * AG Venue Clients (moving between venues, etc.) and provides grav interface
 * elements for access.
 *
 * Created on: Nov 8, 2010
 * @author Andrew Ford
 */

#ifndef VENUECLIENTCONTROLLER_H_
#define VENUECLIENTCONTROLLER_H_

#include <string>
#include <map>

#include "PythonTools.h"
#include "Group.h"

class gravManager;

class VenueClientController : public Group
{

public:
    VenueClientController( float _x, float _y, gravManager* g );

    void draw();

    /*
     * These remove variants remove the member objects entirely (including
     * remove from main lists and delete) since this object effectively owns
     * them.
     */
    void remove( RectangleBase* object, bool move = true );
    std::vector<RectangleBase*>::iterator remove(
                    std::vector<RectangleBase*>::iterator i, bool move = true );

    void getVenueClient();
    void updateExitMap();
    void printExitMap();

    bool updateName();
    void setRendering( bool r );

private:
    std::map<std::string, std::string> exitMap;
    std::string currentVenue;
    std::string venueClientUrl;

    PythonTools pyTools;

    gravManager* grav;

};

#endif /* VENUECLIENTCONTROLLER_H_ */