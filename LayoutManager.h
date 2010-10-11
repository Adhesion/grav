#ifndef LAYOUTMANAGER_H_
#define LAYOUTMANAGER_H_

/**
 * @file LayoutManager.h
 * Contains a set of algorithms, taking in lists of objects and parameters, and
 * arranging the objects in particular ways.
 * Future versions may read from Python scripts.
 * @author Andrew Ford
 *         Ralph Bean
 */

#include <string>
#include <map>
#include <vector>

class RectangleBase;

class LayoutManager
{
    
public:
    LayoutManager();

    void arrange( std::string method,
                  RectangleBase screenRect,
                  RectangleBase boundRect,
                  std::vector<RectangleBase*> objects,
                  std::map<std::string, std::string> options);
    void arrange( std::string method,
                  float screenL, float screenR, float screenU, float screenD,
                  float boundL, float boundR, float boundU, float boundD,
                  std::vector<RectangleBase*> objects,
                  std::map<std::string, std::string> options);

    void perimeterArrange( RectangleBase screenRect, RectangleBase boundRect,
                            std::vector<RectangleBase*> objects );
    void perimeterArrange( float screenL, float screenR, float screenU,
                            float screenD, float boundL, float boundR,
                            float boundU, float boundD,
                            std::vector<RectangleBase*> objects );

    bool gridArrange( RectangleBase boundRect,
                        bool horiz, bool edge, bool resize,
                        std::vector<RectangleBase*> objects,
                        int numX = 0, int numY = 0 );
    bool gridArrange( float boundL, float boundR, float boundU, float boundD,
                        bool horiz, bool edge, bool resize,
                        std::vector<RectangleBase*> objects,
                        int numX = 0, int numY = 0 );

    bool fullscreen( RectangleBase boundRect, RectangleBase* object );
    bool fullscreen( float boundL, float boundR, float boundU, float boundD,
                        RectangleBase* object );
    
    bool focus( RectangleBase boundRect, std::vector<RectangleBase*> outers,
                    std::vector<RectangleBase*> inners,
                    float scaleX = 0.65f, float scaleY = 0.6f );
    bool focus( float boundL, float boundR, float boundU, float boundD,
                    std::vector<RectangleBase*> outers,
                    std::vector<RectangleBase*> inners,
                    float scaleX = 0.65f, float scaleY = 0.6f );

};

#endif /*LAYOUTMANAGER_H_*/
