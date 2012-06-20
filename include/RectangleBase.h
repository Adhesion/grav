/*
 * @file RectangleBase.h
 *
 * RectangleBase defines an object that has position & dimensions and can be
 * drawn, moved, and clicked on. The base class for VideoSource, Group and any
 * future object that needs to be clickable.
 *
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

#ifndef RECTANGLEBASE_H_
#define RECTANGLEBASE_H_

#include <string>

#include "GLUtil.h"
#include "Vector.h"
#include "Ray.h"

typedef struct
{
    float R;
    float G;
    float B;
    float A;
} RGBAColor;

typedef struct
{
    float L;
    float R;
    float U;
    float D;
} Bounds;

enum TextStyle
{
    TOPTEXT,
    CENTEREDTEXT,
    FULLCAPTIONS,
    NOTEXT
};

// we need to do forward declaration since rectanglebase and group circularly
// reference each other
class Group;
class Point;

class RectangleBase
{

public:
    RectangleBase();
    RectangleBase( float _x, float _y );
    RectangleBase( float l, float r, float u, float d );
    RectangleBase( Bounds b );
    RectangleBase( const RectangleBase& other );
    virtual ~RectangleBase();

    void setDefaults();

    /*
     * Returns the width or height of the object. These are virtual because
     * the dimensions of a VideoSource, e.g., are based on the video's aspect
     * ratio in addition to how much it's being scaled. In objects without
     * aspect ratio, these will most likely be equal to the scale factors.
     */
    // TODO fix this dest stuff, it pretty much sucks (ie, make it dynamic so no
    // copy pasted code)
    virtual float getWidth(); virtual float getHeight();
    float getTotalWidth(); float getTotalHeight();
    virtual float getDestWidth(); virtual float getDestHeight();
    float getDestTotalWidth(); float getDestTotalHeight();
    float getBorderSize(); float getDestBorderSize();
    float getBorderScale();

    /*
     * Boundaries of the object, for both current & final positions.
     */
    Bounds getBounds();
    Bounds getDestBounds();
    Bounds getTotalBounds();
    Bounds getDestTotalBounds();

    Vector getNormal();

    /*
     * F is a constant, the same for all points on the plane, used for ray
     * intersection.
     */
    float calculateFVal();

    bool findRayIntersect( Ray r, Point &intersect );

    /*
     * Note that total text height includes offset from border (getTextOffset())
     * Parts of letters that fall under the line (y, g, j, etc) will go in that
     * space.
     */
    float getTextHeight();
    float getTextWidth();
    float getTextScale();
    float getTextOffset();

    float getDestTextHeight();
    float getDestTextWidth();
    float getDestTextScale();
    float getDestTextOffset();

    /*
     * Offset of the center of just the video/inner rectangle content to the
     * center of the whole, meaning the borders and text are included.
     */
    float getCenterOffsetX(); float getCenterOffsetY();
    float getDestCenterOffsetX(); float getDestCenterOffsetY();

    /*
     * Change the position of the object. Move may or may not have animation
     * (based on the animation switch), set will never have animation.
     */
    virtual void move( float _x, float _y, float _z = 0.0f );
    virtual void setPos( float _x, float _y, float z = 0.0f );

    /*
     * Change the scaling of the object.
     */
    virtual void setScale( float xs, float ys );
    /*
     * The bool is so we can have a different version for groups, so they can
     * resize their members or not.
     */
    virtual void setScale( float xs, float ys, bool resizeMembers );

    /*
     * Change the selected dimension to be equal to the argument given,
     * preserving aspect ratio.
     */
    virtual void setWidth( float w );
    virtual void setHeight( float h );

    /*
     * Change the total size, ie, including borders and text, while preserving
     * aspect ratio.
     */
    void setTotalWidth( float w );
    void setTotalHeight( float h );

    /*
     * Change the total size, ie, including borders and text, WITHOUT preserving
     * aspect ratio.
     */
    void setTotalSize( float w, float h );

    /*
     * Set the scale of the border, relative to vertical scaling.
     */
    void setBorderScale( float b );

    /*
     * Sets x, y & z rotation.
     */
    void setRotation( float x, float y, float z );

    /*
     * Fill to another rectangle.
     */
    void fillToRect( RectangleBase rect, bool full = false );
    void fillToRect( float left, float right, float up, float down,
                        bool full = false );

    /*
     * Set the background texture for this object.
     */
    void setTexture( GLuint tex, int width, int height );

    float getX(); float getY(); float getZ();
    float getDestX(); float getDestY(); float getDestZ();
    float getScaleX(); float getScaleY();
    float getLat(); float getLon();

    /*
     * Returns the original aspect ratio of the object. Will be the aspect ratio
     * of the video for a VideoSource, probably 1.0 for everything else.
     */
    virtual float getOriginalAspect();

    void setName( std::string s );
    void setSiteID( std::string sid );
    std::string getName();
    std::string getSubName();
    std::string getAltName();
    std::string getSiteID();

    bool isSelected();
    bool isSelectable();
    void setSelect( bool select );
    virtual void setSelectable( bool s );
    void setEffectVal( float f );
    void setAnimation( bool anim );

    /*
     * Whether the object is movable by the user - controls whether move() calls
     * go through in InputHandler which should be the single entry point.
     */
    bool isUserMovable();

    /*
     * Whether the user can delete this. Right now, only used for test objects.
     */
    bool isUserDeletable();
    void setUserDeletable( bool d );

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

    RGBAColor getColor();
    RGBAColor getBaseColor();
    RGBAColor getSecondaryColor();
    void setColor( RGBAColor c );
    void setBaseColor( RGBAColor c );
    void setSecondaryColor( RGBAColor c );
    /*
     * Resets color back to regular gray color.
     */
    void resetColor();

    /*
     * For now locking only has meaning for groups, but is needed in the generic
     * drawing code.
     */
    bool isLocked();
    void changeLock();

    /*
     * Return whether or not the current name can be said to be 'final'.
     * For video sources, this is the SDES_NAME as opposed to SDES_CNAME, and
     * for groups it signifies whether or not it has figured out a common
     * name from its members.
     */
    bool usingFinalName();

    virtual bool updateName();
    void updateTextBounds();
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

    bool destIntersect( float L, float R, float U, float D );
    bool destIntersect( RectangleBase* other );

    /*
     * Only used for VenueNode override for now.
     */
    virtual void doubleClickAction();

    /*
     * Shows/hides the object. (Hide will cause the alpha animate to 0, and make
     * the object unselectable while it is hidden)
     * Instant will set the color instantly, ie temporarily disable animation
     * for the duration of the method. If you want to create an object that
     * starts as hidden, the best thing to do would be to call setColor() or
     * setBaseColor() and then call show( false, true ) when the object is
     * created.
     */
    virtual void show( bool s, bool instant = false );
    bool isShown();

    /*
     * GL draw function to render the object.
     */
    virtual void draw();
    /*
     * Draw main back texture, assumes position is set up beforehand
     * (ie, no pushmatrix/popmatrix, gltranslate, etc.
     */
    void drawBorder( float Xdist, float Ydist, float s, float t );

protected:
    // position in world space (center of the object)
    float x, y, z;
    // x/y destinations for movement/animation
    float destX, destY, destZ;
    float xAngle, yAngle, zAngle;
    float scaleX, scaleY;
    float destScaleX, destScaleY;
    Vector normal;

    // remember the intended size when resizing so if the aspect ratio changes
    // we can fill the original rectangle properly
    float intendedWidth;
    float intendedHeight;
    // remember whether the last fillToRect had full=true (therefore the
    // intended size represents the inner size, not total size)
    bool lastFillFull;

    // value for the amplitude of the audio connection
    float effectVal;

    // for global positioning
    float lat, lon;

    RGBAColor borderColor;
    RGBAColor destBColor;
    RGBAColor baseBColor;
    RGBAColor secondaryColor;
    RGBAColor destSecondaryColor;

    // save old alpha values for use in show()
    float oldBorderAlpha;
    float oldSecondaryAlpha;

    std::string name;
    std::string altName;
    std::string siteID;
    // substring of the name to render
    int nameStart, nameEnd;
    bool finalName;
    // if name ends up being wider than the object itself, cut off with ellipsis
    int cutoffPos;

    FTBBox textBounds;
    // amount to scale the text relative to the total size
    float relativeTextScale;
    // temp thing for calculating size - possible change to an enum
    TextStyle titleStyle;
    bool coloredText;
    // this is a bool flag for updating the text bounding box the next time the
    // object is drawn - this is because it may do a GL call, which can only be
    // on the main thread
    bool nameSizeDirty;
    // again - this must only be called from the main thread
    void delayedNameSizeUpdate();

    // size of the border relative to total size
    float borderScale;
    GLuint borderTex;
    // width/height of our border/background texture in
    // pixels
    int twidth, theight;

    bool selected;
    bool selectable;
    bool userMovable;
    bool userDeletable;
    bool grouped;
    Group* myGroup;
    bool locked;
    bool showLockStatus;

    bool shown;

    bool debugDraw;

    bool animated;
    virtual void animateValues();

    bool positionAnimating;
    bool scaleAnimating;
    bool borderColAnimating;
    bool secondColAnimating;

};

#endif /*RECTANGLEBASE_H_*/
