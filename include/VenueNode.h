/*
 * VenueNode.h
 *
 * Visual representation of an Access Grid venue.
 *
 * Created on: Nov 11, 2010
 * @author Andrew Ford
 */

#ifndef VENUENODE_H_
#define VENUENODE_H_

#include "RectangleBase.h"

class VenueNode : public RectangleBase
{

public:
    VenueNode();

    void draw();

    bool updateName();

    void doubleClickAction();

private:

};

#endif /* VENUENODE_H_ */
