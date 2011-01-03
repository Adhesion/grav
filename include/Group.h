#ifndef GROUP_H_
#define GROUP_H_

/*
 * @file Group.h
 *
 * Represents a group of objects. The objects contained can be videos or other
 * groups (anything that is a RectangleBase). Objects in the group will be
 * drawn and moved together.
 *
 * @author Andrew Ford
 */

#include "RectangleBase.h"
#include "LayoutManager.h"

#include <vector>

class Group : public RectangleBase
{

public:
    Group( float _x, float _y );
    ~Group();

    virtual void draw();

    void add( RectangleBase* object );
    virtual void remove( RectangleBase* object, bool move = true );
    virtual std::vector<RectangleBase*>::iterator remove(
                    std::vector<RectangleBase*>::iterator i, bool move = true );
    void removeAll();

    RectangleBase* operator[]( int i );
    int numObjects();

    bool isGroup();

    virtual void rearrange();

    virtual bool updateName();

    void move( float _x, float _y );
    void setPos( float _x, float _y );
    void setScale( float xs, float ys );
    void setScale( float xs, float ys, bool resizeMembers );

    void setRendering( bool r );

protected:
    std::vector<RectangleBase*> objects;
    float buffer;
    LayoutManager layouts;

    bool allowHiding;

};

#endif /*GROUP_H_*/
