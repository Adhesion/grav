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

    bool arrange( std::string method,
                  RectangleBase outerRect,
                  RectangleBase innerRect,
                  std::map<std::string, std::vector<RectangleBase*> > data,
                  std::map<std::string, std::string> options=std::map<std::string, std::string>());
    bool arrange( std::string method,
                  float outerL, float outerR, float outerU, float outerD,
                  float innerL, float innerR, float innerU, float innerD,
                  std::map<std::string, std::vector<RectangleBase*> > data,
                  std::map<std::string, std::string> options=std::map<std::string, std::string>());

    // TBD -- consider using a vector<string, vector<Rectangle> > to bring
    //          this under the 'arrange' facade
    bool focus( RectangleBase innerRect, std::vector<RectangleBase*> outers,
                    std::vector<RectangleBase*> inners,
                    float scaleX = 0.65f, float scaleY = 0.6f );
    bool focus( float innerL, float innerR, float innerU, float innerD,
                    std::vector<RectangleBase*> outers,
                    std::vector<RectangleBase*> inners,
                    float scaleX = 0.65f, float scaleY = 0.6f );
private:
    bool perimeterArrange( RectangleBase outerRect, RectangleBase innerRect,
                            std::map<std::string, std::vector<RectangleBase*> > data,
                            std::map<std::string, std::string> options=std::map<std::string, std::string>());
    bool perimeterArrange( float outerL, float outerR, float outerU,
                            float outerD, float innerL, float innerR,
                            float innerU, float innerD,
                            std::map<std::string, std::vector<RectangleBase*> > data,
                            std::map<std::string, std::string> options=std::map<std::string, std::string>());
    bool gridArrange( RectangleBase innerRect,
                        bool horiz, bool edge, bool resize,
                        std::map<std::string, std::vector<RectangleBase*> > data,
                        int numX = 0, int numY = 0 );
    bool gridArrange( float outerL, float outerR, float outerU, float outerD,
                      float innerL, float innerR, float innerU, float innerD,
                      std::map<std::string, std::vector<RectangleBase*> > data,
                      std::map<std::string, std::string> options);
    bool gridArrange( float innerL, float innerR, float innerU, float innerD,
                        bool horiz, bool edge, bool resize,
                        std::map<std::string, std::vector<RectangleBase*> > data,
                        int numX = 0, int numY = 0 );
};

#endif /*LAYOUTMANAGER_H_*/
