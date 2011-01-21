/*
 * Camera.h
 *
 * Represents the camera in 3d space.
 *
 * Created on: Jan 18, 2011
 * @author Andrew Ford
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
    Point getLookat();
    Vector getLookatDir();

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

};

#endif /* CAMERA_H_ */
