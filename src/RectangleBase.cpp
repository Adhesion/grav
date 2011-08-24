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

RectangleBase::RectangleBase( const RectangleBase& other )
{
    x = other.x; y = other.y; z = other.z;
    destX = other.destX; destY = other.destY;
    scaleX = other.scaleX; scaleY = other.scaleY;
    destScaleX = other.destScaleX; destScaleY = other.destScaleY;
    xAngle = other.xAngle; yAngle = other.yAngle; zAngle = other.zAngle;
    normal = other.normal;

    effectVal = other.effectVal;

    lat = other.lat; lon = other.lon;

    borderColor = other.borderColor;
    destBColor = other.destBColor;
    baseBColor = other.baseBColor;
    secondaryColor = other.secondaryColor;
    destSecondaryColor = other.destSecondaryColor;

    enableRendering = other.enableRendering;
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

    enableRendering = true;
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

    //borderTex = PNGLoader::loadPNG( "/home/andrew/work/src/grav/Debug/border.png",
    //                                twidth, theight );
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
    return getWidth() + ( 2.0f * getBorderSize() );
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
    return getDestWidth() + ( 2.0f * getDestBorderSize() );
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

float RectangleBase::getLBound()
{
    return destX - (getDestWidth()/2.0f);
}

float RectangleBase::getRBound()
{
    return destX + (getDestWidth()/2.0f);
}

float RectangleBase::getUBound()
{
    return destY + (getDestHeight()/2.0f);
}

float RectangleBase::getDBound()
{
    return destY - (getDestHeight()/2.0f);
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
    return scaleY * borderScale;
}

float RectangleBase::getDestBorderSize()
{
    return destScaleY * borderScale;
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

void RectangleBase::setBorderScale( float b )
{
    borderScale = b;
}

void RectangleBase::fillToRect( RectangleBase r, bool full )
{
    fillToRect( r.getLBound(), r.getRBound(), r.getUBound(), r.getDBound(),
                    full );
}

void RectangleBase::fillToRect( float innerL, float innerR,
                                float innerU, float innerD, bool full )
{
    float spaceAspect = fabs( ( innerR - innerL ) / ( innerU - innerD ) );

    // full sizes the object such that the inner part of the rect will match
    // the argument rect, full = false sizes it such that the border and text
    // fit in the argument rect (hence using total width/height)
    if ( full )
    {
        float objectAspect = getWidth() / getHeight();

        if ( ( spaceAspect - objectAspect ) > 0.01f )
            setHeight( innerU - innerD );
        else
            setWidth( innerR - innerL );

        move( ( innerR + innerL ) / 2.0f, ( innerU + innerD ) / 2.0f );
    }
    else
    {
        float objectAspect = getTotalWidth() / getTotalHeight();

        if ( ( spaceAspect - objectAspect ) > 0.01f )
            setTotalHeight( innerU - innerD );
        else
            setTotalWidth( innerR - innerL );
        // TODO need to change this if getCenterOffsetX() is ever meaningful
        // and maybe the above one as well
        move( ( innerR + innerL ) / 2.0f,
              ( ( innerU + innerD ) / 2.0f ) - getCenterOffsetY() );
    }
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
    return selectable;
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
    destBColor = c;

    if ( !animated )
        borderColor = destBColor;
    else
        borderColAnimating = true;
}

void RectangleBase::setSecondaryColor( RGBAColor c )
{
    destSecondaryColor = c;

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

    // only change the actual visual color if not selected - otherwise would
    // override and lose the visual indication of selectedness and confuse the
    // user
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
    // find the bounds of this object
    float left = getX() - getWidth()/2;
    float right = getX() + getWidth()/2;
    float bottom = getY() - getHeight()/2;
    float top = getY() + getHeight()/2;

    return !(L > right || R < left || D > top || U < bottom);
}

bool RectangleBase::intersect( RectangleBase* other )
{
    // find the bounds of the other object
    float left = other->getX() - other->getWidth()/2;
    float right = other->getX() + other->getWidth()/2;
    float bottom = other->getY() - other->getHeight()/2;
    float top = other->getY() + other->getHeight()/2;

    return intersect( left, right, top, bottom );
}

void RectangleBase::doubleClickAction()
{
    // do nothing for now
}

void RectangleBase::setRendering( bool r )
{
    enableRendering = r;
}

bool RectangleBase::getRendering()
{
    return enableRendering;
}

void RectangleBase::draw()
{
    // update the text bounds if that function was called - it's here because
    // BBox() may do a GL call which needs to be on the main thread
    if ( nameSizeDirty )
    {
        float oldHeight = getDestTotalHeight();
        float oldOffsetX = getCenterOffsetX();
        float oldOffsetY = getCenterOffsetY();
        cutoffPos = -1;
        textBounds = font->BBox( getSubName().c_str() );
        // only do cutoff if title is at top - so if centered (or other?)
        // display whole name even if it goes out of bounds
        while ( titleStyle == TOPTEXT && getTextWidth() > getWidth() )
        {
            //relativeTextScale = 0.0009 * ( getWidth() / getTextWidth() );
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

        // also, since the text bounds might change, fix the height/pos
        if ( fabs( oldHeight - getDestTotalHeight() ) > 0.001f &&
             ( titleStyle == TOPTEXT || titleStyle == FULLCAPTIONS ) )
        {
            setTotalHeight( oldHeight );
            move( destX - ( getCenterOffsetX() - oldOffsetX ),
                  destY - ( getCenterOffsetY() - oldOffsetY ) );
        }

        nameSizeDirty = false;
    }

    animateValues();

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

        glBegin( GL_QUADS );
        // set the border color
        glColor4f( borderColor.R/2.0f,
                   borderColor.G/1.5f,
                   borderColor.B/1.3f,
                   borderColor.A/3.0f );

        glVertex3f(-Xdist, -Ydist, 0.0);
        glVertex3f(-Xdist, Ydist, 0.0);
        glVertex3f(Xdist, Ydist, 0.0);
        glVertex3f(Xdist, -Ydist, 0.0);

        glEnd();

        glBegin( GL_QUADS );
        // set the border color
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

        glBegin( GL_LINES );

        glColor4f( borderColor.R/3.5f,
                   borderColor.G/1.0f,
                   borderColor.B/2.3f,
                   borderColor.A/2.0f );

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

void RectangleBase::animateValues()
{
    // note the fabs stuff is to snap to the destination, since we'll never
    // actually get there via the division calls due to roundoff errors

    // movement animation
    if ( positionAnimating )
    {
        //gravUtil::logMessage( "RectBase::animate::pos\n" );
        x += ( destX - x ) / 7.5f;
        y += ( destY - y ) / 7.5f;

        if ( fabs( destX - x ) < 0.01f && fabs( destY - y ) < 0.01f )
        {
            x = destX;
            y = destY;
            positionAnimating = false;
            //gravUtil::logMessage( "RectBase::animate::done with pos\n" );
        }
    }

    if ( scaleAnimating )
    {
        //gravUtil::logMessage( "RectBase::animate::scale\n" );
        scaleX += ( destScaleX - scaleX ) / 7.5f;
        scaleY += ( destScaleY - scaleY ) / 7.5f;

        if ( fabs( destScaleX - scaleX ) < 0.01f &&
             fabs( destScaleY - scaleY ) < 0.01f )
        {
            scaleX = destScaleX;
            scaleY = destScaleY;
            scaleAnimating = false;
            //gravUtil::logMessage( "RectBase::animate::done with scale\n" );
        }
    }

    if ( borderColAnimating )
    {
        //gravUtil::logMessage( "RectBase::animate::bordercol\n" );
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
            //gravUtil::logMessage( "RectBase::animate::done with bordercol\n" );
        }
    }

    if ( secondColAnimating )
    {
        //gravUtil::logMessage( "RectBase::animate::secondcol\n" );
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
            //gravUtil::logMessage( "RectBase::animate::done with secondcol\n" );
        }
    }
}
