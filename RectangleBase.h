#ifndef RECTANGLEBASE_H_
#define RECTANGLEBASE_H_

/*
 * @file RectangleBase.h
 * RectangleBase defines an object that has position & dimensions and can be
 * drawn, moved, and clicked on. The base class for VideoSource, Group and any
 * future object that needs to be clickable.
 * @author Andrew Ford
 */

#include <string>

#include <FTGL/ftgl.h>
#include <GL/gl.h>

typedef struct {
    float R;
    float G;
    float B;
    float A;
} RGBAColor;

// we need to do forward declaration since rectanglebase and group circularly
// reference each other
class Group;

class RectangleBase
{

public:
    RectangleBase( float _x, float _y );
    ~RectangleBase();

    /*
     * Returns the width or height of the object. These are virtual because
     * the dimensions of a VideoSource, e.g., are based on the video's aspect
     * ratio in addition to how much it's being scaled. In objects without
     * aspect ratio, these will most likely be equal to the scale factors.
     */
    virtual float getWidth(); virtual float getHeight();
    
    /*
     * Change the position of the object. Move may or may not have animation
     * (based on the animation switch), set will never have animation.
     */
    virtual void move( float _x, float _y );
    virtual void setPos( float _x, float _y );
    
    /*
     * Change the size of the object.
     */
    void setScale( float xs, float ys ); 
    
    float getX(); float getY(); float getZ();
    float getScaleX(); float getScaleY();
    void setName( std::string s );
    std::string getName();
    std::string getSubName();
    bool isSelected();
    void setSelect( bool select );
    
    /*
     * Is this object a member of a group?
     */
    bool isGrouped();
    
    /*
     * Is this object a group itself?
     */
    virtual bool isGroup();
    
    void setGroup( Group* g );
    Group* getGroup();
    
    /*
     * Return whether or not the current name can be said to be 'final'.
     * For video sources, this is the SDES_NAME as opposed to SDES_CNAME, and
     * for groups it signifies whether or not it has figured out a common
     * name from its members.
     */
    bool usingFinalName();
    
    virtual void updateName();
    void setSubstring( int start, int end );
    
    /*
     * Checks whether this object intersects with another rectangle, defined
     * either by its specific points or an existing rectangle object.
     * 
     * @param L,R,U,D   floating-point values that determine the EDGES
     *                  (left, right, up, down) of the rectangle to be
     *                  checked - not the points.
     */
    bool intersect( float L, float R, float U, float D );
    bool intersect( RectangleBase* other );
    
    /*
     * GL draw function to render the object.
     */
    virtual void draw();
    
protected:
    float x,y,z; // position in world space (center of the object)
    float destX, destY; // x/y destinations for movement/animation
    float angle;
    float scaleX, scaleY;
    float destScaleX, destScaleY;
    
    RGBAColor borderColor;
    RGBAColor destBColor;
    RGBAColor baseBColor;
    
    std::string name;
    int nameStart, nameEnd; // substring of the name to render
    bool finalName;
    
    FTFont* font;
    GLuint borderTex;
    
    bool selected;
    bool grouped;
    Group* myGroup;
    
    bool animated;
    void animateValues();
    
    int drawCounter; // keeps track of how many times we've drawn
    
};

#endif /*RECTANGLEBASE_H_*/
