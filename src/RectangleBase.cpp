/*
 * @file RectangleBase.cpp
 * Implementation of the RectangleBase class from RectangleBase.h
 * @author Andrew Ford
 */

#include "RectangleBase.h"
#include "Group.h"
#include "PNGLoader.h"
#include "GLUtil.h"
#include "Point.h"

#include <cmath>
#include <sys/time.h>

RectangleBase::RectangleBase()
{
    setDefaults();
}

RectangleBase::RectangleBase( float _x, float _y )
{
    setDefaults();
    x = -15.0f; y = 15.0f; z = 0.0f;
    destX = _x; destY = _y;
    origX = x; origY = y;

    // This is time in milliseconds
    positionDuration = 600;
    scaleDuration = 600;
    colorDuration = 250;

    positionAnimStart = currentTime();
    scaleAnimStart = currentTime();
    colorAnimStart = currentTime();
    positionAnimating = scaleAnimating = colorAnimating = true;

}

RectangleBase::RectangleBase( const RectangleBase& other )
{
    x = other.x; y = other.y; z = other.z;
    destX = other.destX; destY = other.destY;
    origX = other.origX; origY = other.origY;
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
    origX = x; origY = y;
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

    animated = true;
    finalName = false;
    cutoffPos = -1;
    nameStart = -1; nameEnd = -1;
    name = "";
    altName = "";
    siteID = "";
    myGroup = NULL;
    twidth = 0; theight = 0;
    effectVal = 0.0f;

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
    if ( titleStyle == TOPTEXT || titleStyle == FULLCAPTIONS )
    {
        h += getTextOffset() + getTextHeight();
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

// TODO this is 0 since the text can only be at the top - maybe change this
// later if text can be in multiple positions
float RectangleBase::getCenterOffsetX()
{
    return 0.0f;
}

float RectangleBase::getCenterOffsetY()
{
    float ret = 0.0f;
    if ( titleStyle == TOPTEXT || titleStyle == FULLCAPTIONS )
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
    origX = x;
    if ( !animated ) x = _x;
    destY = _y;
    origY = y;
    if ( !animated ) y = _y;
    positionAnimStart = currentTime();
    positionAnimating = true;
}

void RectangleBase::setPos( float _x, float _y )
{
    destX = _x; origX = _x; x = _x;
    destY = _y; origY = _y; y = _y;
}

void RectangleBase::setScale( float xs, float ys )
{
    destScaleX = xs; destScaleY = ys;
    origScaleX = scaleX; origScaleY = scaleY;

    if ( !animated ) { scaleX = xs; scaleY = ys; }
    scaleAnimStart = currentTime();
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

// TODO make these more generic, maybe if text can be in different places
void RectangleBase::setTotalWidth( float w )
{
    //float newWidth = w * (1.0f - ( 2.0f * getBorderScale() ) );
    float newWidth = w * getWidth() / getTotalWidth();
    setWidth( newWidth );
}

void RectangleBase::setTotalHeight( float h )
{
    //float textRatio = getTextHeight() / getHeight();
    //float newHeight = h * (1.0f - textRatio - ( 2.0f * getBorderScale() ) );
    float newHeight = h * getHeight() / getTotalHeight();
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

float RectangleBase::getOrigX()
{
    return origX;
}

float RectangleBase::getDestX()
{
    return destX;
}

float RectangleBase::getY()
{
    return y;
}

float RectangleBase::getOrigY()
{
    return origY;
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
    origBColor = borderColor;

    if ( !animated )
        borderColor = destBColor;
    colorAnimStart = currentTime();
    colorAnimating = true;
}

void RectangleBase::setSecondaryColor( RGBAColor c )
{
    destSecondaryColor = c;
    origSecondaryColor = secondaryColor;

    if ( !animated )
        secondaryColor = destSecondaryColor;
    colorAnimStart = currentTime();
    colorAnimating = true;
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

// Get the current time in miliseconds
uint32_t RectangleBase::currentTime()
{
    struct timeval  tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000.0;
}

// simple linear interpolation between two points
float RectangleBase::lerp(float x1, float x2, float t)
{
    return x1 + (x2 - x1)*t;
}

void RectangleBase::finalizeAnimation()
{
   origScaleX = destScaleX;
   origScaleY = destScaleY;
   origBColor = destBColor;
   origSecondaryColor = destSecondaryColor;
}

float RectangleBase::bezier(
                std::vector<float>::iterator points_first,
                std::vector<float>::iterator points_last,
                float t)
{
    if ( points_last - points_first == 2 )
    {
        return lerp(*points_first, *(points_last-1), t);
    }

    std::vector<float>::iterator left_first, left_last, right_first, right_last;
    float left, right;

    left_first = points_first;
    left_last = points_last - 1;
    left = bezier(left_first, left_last, t);

    right_first = points_first + 1;
    right_last = points_last;
    right = bezier(right_first, right_last, t);

    return lerp(left, right, t);
}

// returns a list of points that specify a bezier curve.  has some named ones.
std::vector<float> RectangleBase::bezierSpecification(
                float a, float z, std::string name)
{
    std::vector<float> points;
    points.push_back(a);

    // TODO -- play with introducing fancy points here.
    if ( name == "aggressive" ) {
        points.push_back(a + (z-a) * -0.25);
        points.push_back(a + (z-a) *  1.25);
    } else if ( name == "easy" ) {
        points.push_back(a + (z-a) * 0.01);
        points.push_back(a + (z-a) * 0.99);
    } else if ( name == "snap" ) {
        points.push_back(a + (z-a) * 0.01);
    } else if ( name == "wiggle" ) {
        points.push_back(a + (z-a) * 2.5);
        points.push_back(a + (z-a) * 0.0);
        points.push_back(a + (z-a) * 1.5);
        points.push_back(a + (z-a) * 0.9);
    } else if ( name == "none" ) {
        // pass
    } else {
        printf("Got unexpected animation name %s\n", name.c_str());
    }

    points.push_back(z);
    return points;
}

// evaluate a point on a bezier-curve. t goes from 0 to 1.0
void RectangleBase::animateValues()
{
    if ( ! positionAnimating && ! scaleAnimating && ! colorAnimating ) return;

    uint32_t current = currentTime();
    std::vector<float> points;
    std::vector<float> snap = bezierSpecification(0, 1, "snap");

    if ( positionAnimating ) {
        float pt = (float)(current - positionAnimStart)/positionDuration;
        if ( pt > 0.0 && pt < 1.0 ) {
            pt = bezier(snap.begin(), snap.end(), pt);

            points = bezierSpecification(origX, destX, "aggressive");
            x = bezier(points.begin(), points.end(), pt);

            points = bezierSpecification(origY, destY, "easy");
            y = bezier(points.begin(), points.end(), pt);
        } else {
            x = origX = destX;
            y = origY = destY;
            positionAnimating = false;
        }
    }

    if ( scaleAnimating ) {
        float st = (float)(current - scaleAnimStart)/scaleDuration;
        if ( st > 0.0 && st < 1.0 ) {
            st = bezier(snap.begin(), snap.end(), st);

            points = bezierSpecification(origScaleX, destScaleX);
            scaleX = bezier(points.begin(), points.end(), st);

            points = bezierSpecification(origScaleY, destScaleY);
            scaleY = bezier(points.begin(), points.end(), st);
        } else {
            scaleX = origScaleX = destScaleX;
            scaleY = origScaleY = destScaleY;
            scaleAnimating = false;
        }
    }

    if ( colorAnimating ) {
        float ct = (float)(current - colorAnimStart)/colorDuration;
        if ( ct > 0.0 && ct < 1.0 ) {
            points = bezierSpecification(origBColor.R, destBColor.R);
            borderColor.R = bezier(points.begin(), points.end(), ct);
            points = bezierSpecification(origBColor.G, destBColor.G);
            borderColor.G = bezier(points.begin(), points.end(), ct);
            points = bezierSpecification(origBColor.B, destBColor.B);
            borderColor.B = bezier(points.begin(), points.end(), ct);
            points = bezierSpecification(origBColor.A, destBColor.A);
            borderColor.A = bezier(points.begin(), points.end(), ct);

            points = bezierSpecification(
                            origSecondaryColor.R, destSecondaryColor.R);
            secondaryColor.R = bezier(points.begin(), points.end(), ct);
            points = bezierSpecification(
                            origSecondaryColor.G, destSecondaryColor.G);
            secondaryColor.G = bezier(points.begin(), points.end(), ct);
            points = bezierSpecification(
                            origSecondaryColor.B, destSecondaryColor.B);
            secondaryColor.B = bezier(points.begin(), points.end(), ct);
            points = bezierSpecification(
                            origSecondaryColor.A, destSecondaryColor.A);
            secondaryColor.A = bezier(points.begin(), points.end(), ct);
        } else {
            borderColor = origBColor = destBColor;
            secondaryColor = origSecondaryColor = destSecondaryColor;
            colorAnimating = true;
        }
    }
}
