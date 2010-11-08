/*
 * VenueClientController.cpp
 *
 * Implementation of a class for controlling Access Grid Venue Clients.
 *
 * Created on: Nov 8, 2010
 * @author Andrew Ford
 */

#include "VenueClientController.h"

VenueClientController::VenueClientController()
{

}

void VenueClientController::draw()
{

}

void VenueClientController::getVenueClient()
{
    PyObject* pRes = pyTools.call( "py/AGTools.py", "GetVenueClients" );
    std::vector<std::string> venueClients = pyTools.ltov( pRes );
    if ( venueClients.size() == 0 )
    {
        printf( "VenueClientController::getVenueClient(): error: no venue "
                "client found\n" );
    }
    else
    {
        // TODO just take first for now - maybe if multiple found, prompt user?
        venueClientUrl = venueClients[0];
    }
}

void VenueClientController::updateExitMap()
{
    if ( venueClientUrl.compare( "" ) == 0 )
    {
        return;
    }

    exitMap = pyTools.dtom( pyTools.call( "AGTools.py", "GetExits",
                                            venueClientUrl ) );
}

void VenueClientController::printExitMap()
{
    printf( "VenueClientController::printExitMap: from venue client at %s\n",
                venueClientUrl.c_str() );
    std::map<std::string, std::string>::iterator it;
    for ( it = exitMap.begin(); it != exitMap.end(); ++it )
    {
        printf( "%s : %s\n", it->first.c_str(), it->second.c_str() );
    }
}

void VenueClientController::setRendering( bool r )
{

}
