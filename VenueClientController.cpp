/*
 * VenueClientController.cpp
 *
 * Implementation of a class for controlling Access Grid Venue Clients.
 *
 * Created on: Nov 8, 2010
 * @author Andrew Ford
 */

#include "VenueClientController.h"
#include "gravManager.h"
#include "VenueNode.h"

VenueClientController::VenueClientController( float _x, float _y,
                                                gravManager* g )
    : Group( _x, _y ), grav( g )
{
    locked = false;
    selectable = false;
    userMovable = false;
    allowHiding = true;
    setName( "Venues" );

    updateExitMap();
    baseBColor.R = 0.7f;
    baseBColor.G = 0.7f;
    baseBColor.B = 1.0f;
    baseBColor.A = 0.8f;
    destBColor = baseBColor;
    destBColor.A = 0.0f;
    borderColor = destBColor;
    setRendering( false );
    setScale( 15.0f, 15.0f );
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

void VenueClientController::getVenueClient()
{
    PyObject* pRes = pyTools.call( "AGTools", "GetVenueClients" );
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
    Py_XDECREF( pRes );
}

void VenueClientController::updateExitMap()
{
    // if no venue client, try to get; if not then fail
    if ( venueClientUrl.compare( "" ) == 0 )
    {
        getVenueClient();
    }
    if ( venueClientUrl.compare( "" ) == 0 )
    {
        return;
    }

    PyObject* pRes = pyTools.call( "AGTools", "GetExits",
                                    venueClientUrl );
    exitMap = pyTools.dtom( pRes );
    Py_XDECREF( pRes );

    // TODO check if exitMap changes here, to avoid needless remake?
    removeAll();
    std::map<std::string, std::string>::iterator i;
    for ( i = exitMap.begin(); i != exitMap.end(); ++i )
    {
        VenueNode* node = new VenueNode();
        node->setName( i->first );

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
    printf( "VenueClientController::printExitMap: from venue client at %s\n",
                venueClientUrl.c_str() );
    std::map<std::string, std::string>::iterator it;
    for ( it = exitMap.begin(); it != exitMap.end(); ++it )
    {
        printf( "%s : %s\n", it->first.c_str(), it->second.c_str() );
    }

    printf( "Objects, should correspond to venues:\n" );
    for ( unsigned int i = 0; i < objects.size(); i++ )
    {
        printf( "%i: %s\n", i, objects[i]->getSubName().c_str() );
    }
}

bool VenueClientController::updateName()
{
    // like runway, this does nothing
    return false;
}

void VenueClientController::setRendering( bool r )
{
    // TODO this is the same as Runway, should abstract this somehow?
    printf( "VCC::setting rendering to %i\n", r );
    enableRendering = r;
    printExitMap();

    if ( !r )
    {
        destBColor.A = 0.0f;
        // set children to unselected just in case they're selected - user will
        // probably end up moving them accidentally
        for ( unsigned int i = 0; i < objects.size(); i++ )
        {
            objects[i]->setSelect( false );
        }
    }
    else
        destBColor.A = baseBColor.A;

    for ( unsigned int i = 0; i < objects.size(); i++ )
    {
        RGBAColor col = objects[i]->getBaseColor();
        RGBAColor col2 = objects[i]->getSecondaryColor();
        if ( !r )
        {
            col.A = 0.0f;
            col2.A = 0.0f;
        }
        // note secondary color is going to lose its previous alpha here,
        // TODO fix this somehow? adding a base-secondary-color seems excessive
        else
            col2.A = 1.0f;
        objects[i]->setColor( col );
        objects[i]->setSecondaryColor( col2 );
        objects[i]->setSelectable( r );
    }
}
