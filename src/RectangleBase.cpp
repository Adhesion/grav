/*
 * @file RectangleBase.cpp
 *
 * Implementation of the RectangleBase class - defines drawing and sizing
 * methods for a 2D rectangle in 3D space.
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

#include "RectangleBase.h"
#include "Group.h"
#include "PNGLoader.h"
#include "GLUtil.h"
#include "Point.h"

#include "gravUtil.h"

#include <cmath>

RectangleBase::RectangleBase()
{
    setDefaults();
}

RectangleBase::RectangleBase( float _x, float _y )
{
    setDefaults();
    x = -15.0f; y = 15.0f; z = 0.0f;
    move( _x, _y );
}

RectangleBase::RectangleBase( float l, float r, float u, float d )
{
    setDefaults();
    x = -15.0f; y = 15.0f; z = 0.0f;

    setScale( r - l, u - d );
    move( ( r + l ) / 2.0f, ( u + d ) / 2.0f );
}

RectangleBase::RectangleBase( Bounds b )
{
    setDefaults();
    x = -15.0f; y = 15.0f; z = 0.0f;

    setScale( b.R - b.L, b.U - b.D );
    move( ( b.R + b.L ) / 2.0f, ( b.U + b.D ) / 2.0f );
}

RectangleBase::RectangleBase( const RectangleBase& other )
{
    x = other.x; y = other.y; z = other.z;
    destX = other.destX; destY = other.destY;
    scaleX = other.scaleX; scaleY = other.scaleY;
    destScaleX = other.destScaleX; destScaleY = other.destScaleY;
    xAngle = other.xAngle; yAngle = other.yAngle; zAngle = other.zAngle;
    normal = other.normal;

    intendedWidth = other.intendedWidth;
    intendedHeight = other.intendedHeight;
    lastFillFull = other.lastFillFull;

    effectVal = other.effectVal;

    lat = other.lat; lon = other.lon;

    borderColor = other.borderColor;
    destBColor = other.destBColor;
    baseBColor = other.baseBColor;
    secondaryColor = other.secondaryColor;
    destSecondaryColor = other.destSecondaryColor;

    oldBorderAlpha = other.oldBorderAlpha;
    oldSecondaryAlpha = other.oldSecondaryAlpha;

    shown = other.shown;

    debugDraw = other.debugDraw;

    name = other.name;
    siteID = other.siteID;
    nameStart = other.nameStart; nameEnd = other.nameEnd;
    finalName = other.finalName;
    cutoffPos = other.cutoffPos;

    font = other.font;
    relativeTextScale = other.relativeTextScale;
    borderScale = other.borderScale;
    titleStyle = other.titleStyle;
    coloredText = other.coloredText;
    nameSizeDirty = other.nameSizeDirty;

    borderTex = other.borderTex;
    twidth = other.twidth; theight = other.theight;

    selected = other.selected;
    selectable = other.selectable;
    userMovable = other.userMovable;
    userDeletable = other.userDeletable;
    grouped = other.grouped;
    myGroup = other.myGroup;

    animated = other.animated;
    positionAnimating = other.positionAnimating;
    scaleAnimating = other.scaleAnimating;
    borderColAnimating = other.borderColAnimating;
    secondColAnimating = other.secondColAnimating;
}

RectangleBase::~RectangleBase()
{
    if ( isGrouped() )
        myGroup->remove( this );

    // if this is set externally, then we shouldn't delete it since other
    // things might be using it
    //glDeleteTextures( 1, &borderTex );

    // font is not deleted here since the default is to use the global one from
    // GLUtil, and that will delete it
}

void RectangleBase::setDefaults()
{
    scaleX = 5.0f; scaleY = 5.0f;
    xAngle = 0.0f; yAngle = 0.0f; zAngle = 0.0f;
    x = 0.0f; y = 0.0f; z = 0.0f;
    destX = x; destY = y;
    destScaleX = scaleX; destScaleY = scaleY;
    // TODO make this actually based on the rotation
    normal = Vector( 0.0f, 0.0f, 1.0f );

    selected = false;
    selectable = true;
    userMovable = true;
    userDeletable = false;
    showLockStatus = false;
    locked = false;

    shown = true;

    debugDraw = false;

    relativeTextScale = 0.0009;
    titleStyle = TOPTEXT;
    coloredText = true;
    nameSizeDirty = false;

    borderScale = 0.04;

    resetColor();
    borderColor = destBColor;
    destSecondaryColor.R = 0.0f; destSecondaryColor.G = 0.0f;
    destSecondaryColor.B = 0.0f; destSecondaryColor.A = 0.0f;
    secondaryColor = destSecondaryColor;

    oldBorderAlpha = destBColor.A;
    oldSecondaryAlpha = destSecondaryColor.A;

    finalName = false;
    cutoffPos = -1;
    nameStart = -1; nameEnd = -1;
    name = "";
    altName = "";
    siteID = "";
    myGroup = NULL;
    twidth = 0; theight = 0;
    effectVal = 0.0f;

    animated = true;
    positionAnimating = false;
    scaleAnimating = false;
    borderColAnimating = false;
    secondColAnimating = false;

    // TODO: this should be dynamic
    lat = 43.165556f; lon = -77.611389f;

    font = GLUtil::getInstance()->getMainFont();

    intendedWidth = getDestTotalWidth();
    intendedHeight = getDestTotalHeight();
    lastFillFull = false;
}

float RectangleBase::getWidth()
{
    return scaleX;
}

float RectangleBase::getHeight()
{
    return scaleY;
}

float RectangleBase::getTotalWidth()
{
    float w = getWidth() + ( 2.0f * getBorderSize() );
    if ( titleStyle == CENTEREDTEXT )
    {
        return std::max( w, getTextWidth() );
    }
    else
    {
        return w;
    }
}

float RectangleBase::getTotalHeight()
{
    float h = getHeight() + ( 2.0f * getBorderSize() );
    if ( getTextHeight() > 0.0f &&
         ( titleStyle == TOPTEXT || titleStyle == FULLCAPTIONS ) )
    {
        h += getTextOffset() + getTextHeight();
    }
    return h;
}

float RectangleBase::getDestTotalWidth()
{
    float w = getDestWidth() + ( 2.0f * getDestBorderSize() );
    if ( titleStyle == CENTEREDTEXT )
    {
        return std::max( w, getDestTextWidth() );
    }
    else
    {
        return w;
    }
}

float RectangleBase::getDestTotalHeight()
{
    float h = getDestHeight() + ( 2.0f * getDestBorderSize() );
    if ( getDestTextHeight() > 0.0f &&
         ( titleStyle == TOPTEXT || titleStyle == FULLCAPTIONS ) )
    {
        h += getDestTextOffset() + getDestTextHeight();
    }
    return h;
}

float RectangleBase::getDestWidth()
{
    return destScaleX;
}

float RectangleBase::getDestHeight()
{
    return destScaleY;
}

Bounds RectangleBase::getBounds()
{
    Bounds b;
    b.L = x - (getWidth()/2.0f);
    b.R = x + (getWidth()/2.0f);
    b.U = y + (getHeight()/2.0f);
    b.D = y - (getHeight()/2.0f);
    return b;
}

Bounds RectangleBase::getDestBounds()
{
    Bounds b;
    b.L = destX - (getDestWidth()/2.0f);
    b.R = destX + (getDestWidth()/2.0f);
    b.U = destY + (getDestHeight()/2.0f);
    b.D = destY - (getDestHeight()/2.0f);
    return b;
}

Bounds RectangleBase::getTotalBounds()
{
    Bounds b;
    b.L = x - (getTotalWidth()/2.0f) + getCenterOffsetX();
    b.R = x + (getTotalWidth()/2.0f) + getCenterOffsetX();
    b.U = y + (getTotalHeight()/2.0f) + getCenterOffsetY();
    b.D = y - (getTotalHeight()/2.0f) + getCenterOffsetY();
    return b;
}

Bounds RectangleBase::getDestTotalBounds()
{
    Bounds b;
    b.L = destX - (getDestTotalWidth()/2.0f) + getDestCenterOffsetX();
    b.R = destX + (getDestTotalWidth()/2.0f) + getDestCenterOffsetX();
    b.U = destY + (getDestTotalHeight()/2.0f) + getDestCenterOffsetY();
    b.D = destY - (getDestTotalHeight()/2.0f) + getDestCenterOffsetY();
    return b;
}

Vector RectangleBase::getNormal()
{
    return normal;
}

float RectangleBase::calculateFVal()
{
    return -( normal.getX() * x + normal.getY() * y + normal.getZ() * z);
}

bool RectangleBase::findRayIntersect( Ray r, Point& intersect )
{
    float dot = r.direction.dotProduct( normal );

    if ( dot >= 0 )
    {
        return false;
    }

    float w = -( normal.dotProduct( r.location.toVector() ) + calculateFVal() )
                   / dot;

    intersect.setX( r.location.getX() + ( r.direction.getX() * w ) );
    intersect.setY( r.location.getY() + ( r.direction.getY() * w ) );
    intersect.setZ( r.location.getZ() + ( r.direction.getZ() * w ) );
    // TODO also need to check whether intersect is actually on the polygon
    // right now, it just checks whether it's on the plane

    return true;
}

float RectangleBase::getBorderSize()
{
    return scaleX * borderScale;
}

float RectangleBase::getDestBorderSize()
{
    return destScaleX * borderScale;
}

float RectangleBase::getBorderScale()
{
    return borderScale;
}

float RectangleBase::getTextHeight()
{
    // this ignores textBounds.Lower(), since 0.0 in its coordinate system is
    // the baseline
    return textBounds.Upper().Yf() * getTextScale();
}

float RectangleBase::getTextWidth()
{
    return ( textBounds.Upper().Xf() - textBounds.Lower().Xf() )
            * getTextScale();
}

float RectangleBase::getTextScale()
{
    return scaleX * relativeTextScale;
}

float RectangleBase::getTextOffset()
{
    return getBorderSize() * 0.4f;
}

float RectangleBase::getDestTextHeight()
{
    // this ignores textBounds.Lower(), since 0.0 in its coordinate system is
    // the baseline
    return textBounds.Upper().Yf() * getDestTextScale();
}

float RectangleBase::getDestTextWidth()
{
    return ( textBounds.Upper().Xf() - textBounds.Lower().Xf() )
            * getDestTextScale();
}

float RectangleBase::getDestTextScale()
{
    return destScaleX * relativeTextScale;
}

float RectangleBase::getDestTextOffset()
{
    return getDestBorderSize() * 0.4f;
}

// TODO this is 0 since the text can only be at the top - maybe change this
// later if text can be in multiple positions
float RectangleBase::getCenterOffsetX()
{
    return 0.0f;
}

float RectangleBase::getCenterOffsetY()
{
    float ret = 0.0f;
    if ( getTextHeight() > 0.0f &&
         ( titleStyle == TOPTEXT || titleStyle == FULLCAPTIONS ) )
    {
        float textRatio = ( getTextHeight() + getTextOffset() ) / getHeight();
        ret = textRatio * getDestHeight() / 2.0f;
        if ( titleStyle == FULLCAPTIONS )
            ret *= -1.0f;
    }
    return ret;
}

// TODO see above
float RectangleBase::getDestCenterOffsetX()
{
    return 0.0f;
}

float RectangleBase::getDestCenterOffsetY()
{
    float ret = 0.0f;
    if ( getDestTextHeight() > 0.0f &&
         ( titleStyle == TOPTEXT || titleStyle == FULLCAPTIONS ) )
    {
        float textRatio = ( getDestTextHeight() + getDestTextOffset() ) /
                getDestHeight();
        ret = textRatio * getDestHeight() / 2.0f;
        if ( titleStyle == FULLCAPTIONS )
            ret *= -1.0f;
    }
    return ret;
}

void RectangleBase::move( float _x, float _y )
{
    destX = _x;
    destY = _y;
    if ( !animated )
    {
        x = _x;
        y = _y;
    }
    else
        positionAnimating = true;

}

void RectangleBase::setPos( float _x, float _y )
{
    destX = _x; x = _x;
    destY = _y; y = _y;
}

void RectangleBase::setScale( float xs, float ys )
{
    destScaleX = xs; destScaleY = ys;
    if ( !animated )
    {
        scaleX = xs;
        scaleY = ys;
    }
    else
        scaleAnimating = true;

    intendedWidth = getDestTotalWidth();
    intendedHeight = getDestTotalHeight();
    lastFillFull = false;
}

void RectangleBase::setScale( float xs, float ys, bool resizeMembers )
{
    setScale( xs, ys );
}

void RectangleBase::setWidth( float w )
{
    setScale( w, destScaleY * w / destScaleX );
}

void RectangleBase::setHeight( float h )
{
    setScale( destScaleX * h / destScaleY, h );
}

void RectangleBase::setTotalWidth( float w )
{
    float newWidth = w * getDestWidth() / getDestTotalWidth();
    setWidth( newWidth );
}

void RectangleBase::setTotalHeight( float h )
{
    float newHeight = h * getDestHeight() / getDestTotalHeight();
    setHeight( newHeight );
}

/*
 * Note how this method replicates the definition of height/width as far as
 * border size & text go - so if those change this needs to as well.
 * Not sure if there's a way around this.
 */
void RectangleBase::setTotalSize( float w, float h )
{
    // we need to set X first since border, text etc depend on it...
    // (using the original aspect here will correctly size videos since their
    // width calculation is based on that as well)
    float newX = w / ( getOriginalAspect() + ( 2 * borderScale ) );
    setScale( newX, destScaleY );

    // ...then we can figure out Y based on those values
    float newY = h - ( 2 * getDestBorderSize() );
    if ( getDestTextHeight() > 0.0f &&
        ( titleStyle == TOPTEXT || titleStyle == FULLCAPTIONS ) )
    {
        newY -= getDestTextOffset() + getDestTextHeight();
    }

    setScale( newX, newY );
}

void RectangleBase::setBorderScale( float b )
{
    borderScale = b;
}

void RectangleBase::fillToRect( RectangleBase r, bool full )
{
    float spaceAspect = r.getDestWidth() / r.getDestHeight();

    // full sizes the object such that the inner part of the rect will match
    // the argument rect, full = false sizes it such that the border and text
    // fit in the argument rect (hence using total width/height)
    if ( full )
    {
        float objectAspect = getDestWidth() / getDestHeight();

        if ( ( spaceAspect - objectAspect ) > 0.01f )
            setHeight( r.getDestHeight() );
        else
            setWidth( r.getDestWidth() );

        move( r.getDestX(), r.getDestY() );
    }
    else
    {
        float objectAspect = getDestTotalWidth() / getDestTotalHeight();

        if ( ( spaceAspect - objectAspect ) > 0.01f )
            setTotalHeight( r.getDestHeight() );
        else
            setTotalWidth( r.getDestWidth() );

        // TODO need to change this if getCenterOffsetX() is ever meaningful
        // and maybe the above one as well
        move( r.getDestX(), r.getDestY() - getCenterOffsetY() );
    }

    // override setting intended bounds in setscale, since we know the input
    // to this function is what we want the size to be ideally
    intendedWidth = r.getDestWidth();
    intendedHeight = r.getDestHeight();
    lastFillFull = full;
}

void RectangleBase::fillToRect( float innerL, float innerR,
                                float innerU, float innerD, bool full )
{
    fillToRect( RectangleBase( innerL, innerR, innerU, innerD ), full );
}

void RectangleBase::setTexture( GLuint tex, int width, int height )
{
    borderTex = tex;
    twidth = width; theight = height;
}

float RectangleBase::getX()
{
    return x;
}

float RectangleBase::getDestX()
{
    return destX;
}

float RectangleBase::getY()
{
    return y;
}

float RectangleBase::getDestY()
{
    return destY;
}

float RectangleBase::getZ()
{
    return z;
}

float RectangleBase::getScaleX()
{
    return scaleX;
}

float RectangleBase::getScaleY()
{
    return scaleY;
}

float RectangleBase::getLat()
{
    return lat;
}

float RectangleBase::getLon()
{
    return lon;
}

float RectangleBase::getOriginalAspect()
{
    return 1.0;
}

void RectangleBase::setName( std::string s )
{
    bool nameChanged = s.compare( name ) == 0;
    name = s;
    updateTextBounds();
}

void RectangleBase::setSiteID( std::string sid )
{
    siteID = sid;
}

std::string RectangleBase::getName()
{
    return name;
}

std::string RectangleBase::getSubName()
{
    if ( nameStart != -1 && nameEnd != -1 )
    {
        if ( cutoffPos == -1 )
            return name.substr( nameStart, nameEnd - nameStart );
        else
            return name.substr( nameStart, cutoffPos - nameStart );
    }
    else
        return name;
}

std::string RectangleBase::getAltName()
{
    return altName;
}

std::string RectangleBase::getSiteID()
{
    return siteID;
}

bool RectangleBase::isSelected()
{
    return selected;
}

bool RectangleBase::isSelectable()
{
    return selectable && shown;
}

void RectangleBase::setSelect( bool select )
{
    selected = select;
    if ( select )
    {
        RGBAColor selectCol;
        selectCol.R = 1.0f; selectCol.G = 1.0f;
        selectCol.B = 0.0f, selectCol.A = 0.8f;

        setColor( selectCol );
    }
    else
    {
        setColor( baseBColor );
    }
}

void RectangleBase::setSelectable( bool s )
{
    selectable = s;
}

void RectangleBase::setEffectVal( float f )
{
    effectVal = f;
}

void RectangleBase::setAnimation( bool anim )
{
    animated = anim;
}

bool RectangleBase::isUserMovable()
{
    return userMovable;
}

bool RectangleBase::isUserDeletable()
{
    return userDeletable;
}

void RectangleBase::setUserDeletable( bool d )
{
    userDeletable = d;
}

bool RectangleBase::isGrouped()
{
    return ( myGroup != NULL );
}

bool RectangleBase::isGroup()
{
    return false;
}

void RectangleBase::setGroup( Group* g )
{
    myGroup = g;
    if ( g == NULL )
        updateName();
}

Group* RectangleBase::getGroup()
{
    return myGroup;
}

RGBAColor RectangleBase::getColor()
{
    return borderColor;
}

RGBAColor RectangleBase::getBaseColor()
{
    return baseBColor;
}

RGBAColor RectangleBase::getSecondaryColor()
{
    return secondaryColor;
}

void RectangleBase::setColor( RGBAColor c )
{
    if ( shown )
        destBColor = c;
    else
    {
        destBColor.R = c.R;
        destBColor.G = c.G;
        destBColor.B = c.B;
        oldBorderAlpha = c.A;
    }

    if ( !animated )
        borderColor = destBColor;
    else
        borderColAnimating = true;
}

void RectangleBase::setBaseColor( RGBAColor c )
{
    baseBColor = c;

    // only change the actual visual color if not selected - otherwise would
    // override and lose the visual indication of selectedness and confuse the
    // user
    if ( !selected )
        setColor( baseBColor );
}

void RectangleBase::setSecondaryColor( RGBAColor c )
{
    if ( shown )
        destSecondaryColor = c;
    else
    {
        destSecondaryColor.R = c.R;
        destSecondaryColor.G = c.G;
        destSecondaryColor.B = c.B;
        oldSecondaryAlpha = c.A;
    }

    if ( !animated )
        secondaryColor = destSecondaryColor;
    else
        secondColAnimating = true;
}

void RectangleBase::resetColor()
{
    RGBAColor original;
    original.R = 1.0f; original.G = 1.0f;
    original.B = 1.0f; original.A = 0.7f;
    baseBColor = original;

    // like setBaseColor, defer color change to when deselected
    if ( !selected )
        setColor( original );
}

bool RectangleBase::isLocked()
{
    return locked;
}

void RectangleBase::changeLock()
{
    locked = !locked;
}

bool RectangleBase::usingFinalName()
{
    return finalName;
}

bool RectangleBase::updateName()
{
    return false;
}

void RectangleBase::updateTextBounds()
{
    if ( font )
    {
        nameSizeDirty = true;
    }
}

void RectangleBase::setSubstring( int start, int end )
{
    nameStart = start;
    nameEnd = end;
    // reset cutoff, since it'll confirm again when updatetext is called
    updateTextBounds();
}

bool RectangleBase::intersect( float L, float R, float U, float D )
{
    Bounds b = getBounds();
    return !( L > b.R || R < b.L || D > b.U || U < b.D );
}

bool RectangleBase::intersect( RectangleBase* other )
{
    Bounds ob = other->getBounds();
    return intersect( ob.L, ob.R, ob.U, ob.D );
}

bool RectangleBase::destIntersect( float L, float R, float U, float D )
{
    Bounds b = getDestBounds();
    return !( L > b.R || R < b.L || D > b.U || U < b.D );
}

bool RectangleBase::destIntersect( RectangleBase* other )
{
    Bounds ob = other->getDestBounds();
    return destIntersect( ob.L, ob.R, ob.U, ob.D );
}

void RectangleBase::doubleClickAction()
{
    // do nothing for now
}

void RectangleBase::show( bool s, bool instant )
{
    bool wasAnimated = animated;
    if ( instant && wasAnimated )
        animated = false;

    float newAlpha, newSecondAlpha;

    if ( !s )
    {
        if ( selected )
            setSelect( false );

        newAlpha = 0.0f;
        newSecondAlpha = 0.0f;
        oldBorderAlpha = destBColor.A;
        oldSecondaryAlpha = destSecondaryColor.A;
    }
    else
    {
        newAlpha = oldBorderAlpha;
        newSecondAlpha = oldSecondaryAlpha;
    }

    destBColor.A = newAlpha;
    destSecondaryColor.A = newSecondAlpha;

    if ( !animated )
    {
        borderColor.A = destBColor.A;
        secondaryColor.A = destSecondaryColor.A;
    }
    else
    {
        borderColAnimating = true;
        secondColAnimating = true;
    }

    // this is done late since setSelect( false ) above will call setcolor
    // which depends on the shown bool being accurate for the current state
    shown = s;

    if ( instant && wasAnimated )
        animated = true;
}

bool RectangleBase::isShown()
{
    return shown;
}

void RectangleBase::draw()
{
    // update the text bounds if that function was called - it's here because
    // BBox() may do a GL call which needs to be on the main thread
    if ( nameSizeDirty )
    {
        delayedNameSizeUpdate();
    }

    animateValues();

    if ( borderColor.A < 0.01f )
        return;

    // set up our position
    glPushMatrix();

    glTranslatef( x, y, z );

    glRotatef( xAngle, 1.0, 0.0, 0.0 );
    glRotatef( yAngle, 0.0, 1.0, 0.0 );
    glRotatef( zAngle, 0.0, 0.0, 1.0 );

    // draw the border first
    float s = (float)twidth / (float)GLUtil::getInstance()->pow2( twidth );
    float t = (float)theight / (float)GLUtil::getInstance()->pow2( theight );

    // X & Y distances from center to edge
    float Xdist = (getWidth()/2.0f) + getBorderSize();
    float Ydist = (getHeight()/2.0f) + getBorderSize();

    // DEBUG DRAW
    if ( debugDraw )
    {
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

        // draw intended bounds
        glBegin( GL_QUADS );

        glColor4f( borderColor.R/2.0f,
                   borderColor.G/2.0f,
                   borderColor.B/2.0f,
                   borderColor.A/2.0f );

        float iXdist = intendedWidth / 2.0f;
        float iYdist = intendedHeight / 2.0f;
        float Xoff = getCenterOffsetX();
        float Yoff = getCenterOffsetY();
        glVertex3f(-iXdist + Xoff, iYdist + Yoff, 0.0);
        glVertex3f(iXdist + Xoff, iYdist + Yoff, 0.0);
        glVertex3f(iXdist + Xoff, -iYdist + Yoff, 0.0);
        glVertex3f(-iXdist + Xoff, -iYdist + Yoff, 0.0);

        glEnd();

        // draw border box
        glBegin( GL_QUADS );

        glColor4f( borderColor.R/2.5f,
                   borderColor.G/2.5f,
                   borderColor.B/2.5f,
                   borderColor.A/2.5f );

        glVertex3f(-Xdist, -Ydist, 0.0);
        glVertex3f(-Xdist, Ydist, 0.0);
        glVertex3f(Xdist, Ydist, 0.0);
        glVertex3f(Xdist, -Ydist, 0.0);

        glEnd();

        // draw main box
        glBegin( GL_QUADS );

        glColor4f( borderColor.R/1.25f,
                   borderColor.G/1.25f,
                   borderColor.B/1.25f,
                   borderColor.A/1.25f );

        float Xdist2 = (getWidth()/2.0f);
        float Ydist2 = (getHeight()/2.0f);

        glVertex3f(-Xdist2, -Ydist2, 0.0);
        glVertex3f(-Xdist2, Ydist2, 0.0);
        glVertex3f(Xdist2, Ydist2, 0.0);
        glVertex3f(Xdist2, -Ydist2, 0.0);

        glEnd();

        // draw text box
        glBegin( GL_QUADS );

        glColor4f( borderColor.R/1.0f,
                   borderColor.G/3.5f,
                   borderColor.B/2.3f,
                   borderColor.A/3.0f );

        float textLY = (getHeight()/2.0f) + getBorderSize() + getTextOffset();
        float textUY = textLY + getTextHeight();

        glVertex3f(-Xdist, textLY, 0.0);
        glVertex3f(-Xdist, textUY, 0.0);
        glVertex3f(Xdist, textUY, 0.0);
        glVertex3f(Xdist, textLY, 0.0);

        glEnd();

        // draw x/y axes lines
        glBegin( GL_LINES );

        glColor4f( borderColor.R/3.5f,
                   borderColor.G/1.0f,
                   borderColor.B/2.3f,
                   borderColor.A/1.3f );

        glVertex3f(-Xdist, 0.0, 0.0);
        glVertex3f(Xdist, 0.0, 0.0);

        glVertex3f(0.0, Ydist, 0.0);
        glVertex3f(0.0, -Ydist, 0.0);

        glEnd();

        glDisable( GL_BLEND );
    }

    drawBorder( Xdist, Ydist, s, t );

    glPushMatrix();

    float textYPos = 0.0f;
    float textXPos = 0.0f;
    if ( titleStyle == TOPTEXT )
    {
        textXPos = -getWidth() / 2.0f;
        textYPos = ( getHeight() / 2.0f ) + getBorderSize() + getTextOffset();
    }
    else if ( titleStyle == CENTEREDTEXT )
    {
        textXPos = -getTextWidth() / 2.0f;
        textYPos = -getTextHeight() / 2.0f;
    }
    else if ( titleStyle == FULLCAPTIONS )
    {
        textXPos = -getTextWidth() / 2.0f;
        textYPos = ( -getHeight() / 2.0f ) - getBorderSize() - getTextOffset() -
                    getTextHeight();
    }

    float scaleFactor = getTextScale();

    /*if ( isGroup() )
    {
        yOffset += getTextHeight();
        scaleFactor *= 0.75f;
    }*/

    glTranslatef( textXPos, textYPos, 0.0f );
    //glRasterPos2f( -getWidth()/2.0f, getHeight()/2.0f+yOffset );
    glScalef( scaleFactor, scaleFactor, scaleFactor );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glEnable( GL_LINE_SMOOTH );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

    if ( font )
    {
        std::string renderedName = getSubName();

        if ( cutoffPos != -1 )
        {
            renderedName += "...";
        }

        if ( showLockStatus && !locked )
            renderedName += " (unlocked)";

        if ( debugDraw )
        {
            char* posString = new char[ 50 ];
            sprintf( posString, "(%f,%f)", x, y );
            renderedName += posString;
        }

        // color call from before will carry over otherwise
        if ( !coloredText )
            glColor4f( 1.0f, 1.0f, 1.0f, borderColor.A );

        const char* nc = renderedName.c_str();
        font->Render(nc);
    }

    glDisable( GL_BLEND );
    glDisable( GL_LINE_SMOOTH );

    glPopMatrix();

    glPopMatrix(); // from initial position setup

    /*float spacing = 0.20f;
    //float i = -name.length()*spacing/2.0f;
    float i = -getWidth();
    for (; *nc != '\0'; nc++)
    {
        glRasterPos2f( i*spacing, getHeight()/2.0f+0.2f );
        //glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_10,*nc);
        i++;
    }
    //glutStrokeString(GLUT_STROKE_MONO_ROMAN, uc);*/
    /*zAngle += 0.5f;
    yAngle += 0.1f;
    xAngle += 0.01f;*/
}

void RectangleBase::drawBorder( float Xdist, float Ydist, float s, float t )
{
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, borderTex );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glPixelStorei( GL_UNPACK_ROW_LENGTH, twidth );

    glBegin( GL_QUADS );
    // set the border color
    glColor4f( borderColor.R-(effectVal*3.0f),
               borderColor.G-(effectVal*3.0f),
               borderColor.B+(effectVal*6.0f),
               borderColor.A+(effectVal*3.0f) );

    glTexCoord2f(0.0, 0.0);
    glVertex3f(-Xdist, -Ydist, 0.0);

    glTexCoord2f(0.0, t);
    glVertex3f(-Xdist, Ydist, 0.0);

    glTexCoord2f(s, t);
    glVertex3f(Xdist, Ydist, 0.0);

    glTexCoord2f(s, 0.0);
    glVertex3f(Xdist, -Ydist, 0.0);

    glEnd();
    glDisable( GL_BLEND );
    glDisable( GL_TEXTURE_2D );
}

/*
 * This must ONLY be called from the main thread. See .h for reason.
 */
void RectangleBase::delayedNameSizeUpdate()
{
    // get old bounds of this to compare size
    RectangleBase intended;
    intended.setScale( intendedWidth, intendedHeight );
    float posX = getDestX();
    float posY = getDestY();
    if ( !lastFillFull )
    {
        posX += getCenterOffsetX();
        posY += getCenterOffsetY();
    }
    intended.setPos( posX, posY );

    cutoffPos = -1;
    textBounds = font->BBox( getSubName().c_str() );
    // only do cutoff if title is at top - so if centered (or other?)
    // display whole name even if it goes out of bounds
    while ( titleStyle == TOPTEXT && getTextWidth() > getWidth() )
    {
        if ( nameStart == -1 || nameEnd == -1 )
        {
            nameStart = 0;
            nameEnd = getName().length();
        }

        int curEnd;
        if ( cutoffPos == -1 )
            curEnd = nameEnd;
        else
            curEnd = cutoffPos;
        int numChars = curEnd - nameStart;

        cutoffPos = curEnd - ceil( ( 1.0f -
              ( getWidth() / getTextWidth() ) ) * numChars ) - 1;
        textBounds = font->BBox( getSubName().c_str() );
    }

    // also, since the text bounds might change, resize to fill the intended
    // rectangle
    fillToRect( intended, lastFillFull );

    nameSizeDirty = false;
}

void RectangleBase::animateValues()
{
    // note the fabs stuff is to snap to the destination, since we'll never
    // actually get there via the division calls due to roundoff errors

    // movement animation
    if ( positionAnimating )
    {
        x += ( destX - x ) / 7.5f;
        y += ( destY - y ) / 7.5f;

        if ( fabs( destX - x ) < 0.01f && fabs( destY - y ) < 0.01f )
        {
            x = destX;
            y = destY;
            positionAnimating = false;
        }
    }

    if ( scaleAnimating )
    {
        scaleX += ( destScaleX - scaleX ) / 7.5f;
        scaleY += ( destScaleY - scaleY ) / 7.5f;

        if ( fabs( destScaleX - scaleX ) < 0.01f &&
             fabs( destScaleY - scaleY ) < 0.01f )
        {
            scaleX = destScaleX;
            scaleY = destScaleY;
            scaleAnimating = false;
        }
    }

    if ( borderColAnimating )
    {
        borderColor.R += ( destBColor.R - borderColor.R ) / 3.0f;
        borderColor.G += ( destBColor.G - borderColor.G ) / 3.0f;
        borderColor.B += ( destBColor.B - borderColor.B ) / 3.0f;
        borderColor.A += ( destBColor.A - borderColor.A ) / 7.0f;

        if ( fabs( destBColor.R - borderColor.R ) < 0.01f &&
             fabs( destBColor.G - borderColor.G ) < 0.01f &&
             fabs( destBColor.B - borderColor.B ) < 0.01f &&
             fabs( destBColor.A - borderColor.A ) < 0.01f )
        {
            borderColor.R = destBColor.R;
            borderColor.G = destBColor.G;
            borderColor.B = destBColor.B;
            borderColor.A = destBColor.A;
            borderColAnimating = false;
        }
    }

    if ( secondColAnimating )
    {
        secondaryColor.R += ( destSecondaryColor.R - secondaryColor.R ) / 3.0f;
        secondaryColor.G += ( destSecondaryColor.G - secondaryColor.G ) / 3.0f;
        secondaryColor.B += ( destSecondaryColor.B - secondaryColor.B ) / 3.0f;
        secondaryColor.A += ( destSecondaryColor.A - secondaryColor.A ) / 7.0f;

        if ( fabs( destSecondaryColor.R - secondaryColor.R ) < 0.01f &&
             fabs( destSecondaryColor.G - secondaryColor.G ) < 0.01f &&
             fabs( destSecondaryColor.B - secondaryColor.B ) < 0.01f &&
             fabs( destSecondaryColor.A - secondaryColor.A ) < 0.01f )
        {
            secondaryColor.R = destSecondaryColor.R;
            secondaryColor.G = destSecondaryColor.G;
            secondaryColor.B = destSecondaryColor.B;
            secondaryColor.A = destSecondaryColor.A;
            secondColAnimating = false;
        }
    }
}
