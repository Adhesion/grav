#ifndef RUNWAY_H_
#define RUNWAY_H_

/*
 * @file Runway.h
 *
 * A group object that holds incoming objects before the user arranges them
 * themselves.
 *
 * Created on: Apr 28, 2010
 * @author Andrew Ford
 */

#include "Group.h"

class Runway : public Group
{

public:
    Runway( float _x, float _y );

    void draw();

    void rearrange();

    bool updateName();

private:
    // 0 means horizontal, 1 means vertical
    int orientation;

};

#endif /* RUNWAY_H_ */
