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
#include "RectangleBase.h"

class VenueClientController : public RectangleBase
{

public:
    VenueClientController();

    void draw();

    void getVenueClient();
    void updateExitMap();
    void printExitMap();
    void setRendering( bool r );

private:
    std::map<std::string, std::string> exitMap;
    std::string currentVenue;
    std::string venueClientUrl;

    PythonTools pyTools;

};

#endif /* VENUECLIENTCONTROLLER_H_ */
