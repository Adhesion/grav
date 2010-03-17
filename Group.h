#ifndef GROUP_H_
#define GROUP_H_

/*
 * @file Group.h
 * Represents a group of objects. The objects contained can be videos or other
 * groups (anything that is a RectangleBase). Objects in the group will be
 * drawn and moved together.
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
    
    void draw();
    
    void add( RectangleBase* object );
    void remove( RectangleBase* object, bool move = true );
    void removeAll();
    
    RectangleBase* operator[]( int i );
    int numObjects();
    
    bool isGroup();
    bool isLocked();
    
    void changeLock();
    
    void rearrange();
    
    void updateName();
    
    void move( float _x, float _y );
    void setPos( float _x, float _y );
    void setScale( float xs, float ys );
    void setScale( float xs, float ys, bool resizeMembers );
    
private:
    std::vector<RectangleBase*> objects;
    float buffer;
    bool locked;
    LayoutManager layouts;
    
};

#endif /*GROUP_H_*/
