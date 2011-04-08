/*
 * @file Camera.h
 *
 * Represents the camera in 3d space.
 *
 * Created on: Jan 18, 2011
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

#ifndef CAMERA_H_
#define CAMERA_H_

#include "Point.h"
#include "Vector.h"
class Earth;

class Camera
{

public:
    Camera( Point c, Point l );
    void doGLLookat();

    Point getCenter();
    Point getDestCenter();
    Point getLookat();
    Point getDestLookat();
    Vector getLookatDir();
    Vector getDestLookatDir();

    /*
     * Set always snaps, move may animate. Similar to RectangleBase.
     * x/y/z is destination point.
     * TODO abstract that to 'movable' or similar to avoid duplicate code
     */
    void setCenter( float x, float y, float z );
    void setCenter( Point p );
    void moveCenter( float x, float y, float z );
    void moveCenter( Point p );

    void setLookat( float x, float y, float z );
    void setLookat( Point p );
    void moveLookat( float x, float y, float z );
    void moveLookat( Point p );

    void setEarth( Earth* e );

    void animateValues();

private:
    Point center;
    Point destCenter;
    Point lookat;
    Point destLookat;
    Vector up;
    Vector destUp;

    // cam has a reference to earth so it can update its up-down rotation axis
    // based on the cam position
    Earth* earth;

    bool animated;
    bool centerMoving;
    bool lookatMoving;

};

#endif /* CAMERA_H_ */
