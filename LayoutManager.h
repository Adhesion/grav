#ifndef LAYOUTMANAGER_H_
#define LAYOUTMANAGER_H_

/**
 * @file LayoutManager.h
 * Contains a set of algorithms, taking in lists of objects and parameters, and
 * arranging the objects in particular ways.
 * Future versions may read from Python scripts.
 * @author Andrew Ford
 */

#include <vector>

class RectangleBase;

class LayoutManager
{
    
public:
    LayoutManager();
    
    void perimeterArrange( float screenL, float screenR, float screenU,
                            float screenD,
                            float boundL, float boundR, float boundU,
                            float boundD, std::vector<RectangleBase*> objects );
    bool gridArrange( float boundL, float boundR, float boundU, float boundD,
                        int numX, int numY, bool horiz,
                        std::vector<RectangleBase*> objects );
    bool fullscreen( float boundL, float boundR, float boundU, float boundD,
                        RectangleBase* object );
    
};

#endif /*LAYOUTMANAGER_H_*/
