/*
 * @file RectangleBase.cpp
 * Implementation of the RectangleBase class from RectangleBase.h
 * @author Andrew Ford
 */

#include "RectangleBase.h"
#include "Group.h"
#include "PNGLoader.h"
#include "GLUtil.h"

#include <cmath>

RectangleBase::RectangleBase()
{
    setDefaults();
}

RectangleBase::RectangleBase( float _x, float _y )
{
    setDefaults();
    x = -15.0f; y = 15.0f; z = 0.0f;
    destX = _x; destY = _y;
}

RectangleBase::RectangleBase( const RectangleBase& other )
{
    x = other.x; y = other.y; z = other.z;
    destX = other.destX; destY = other.destY;
    scaleX = other.scaleX; scaleY = other.scaleY;
    destScaleX = other.destScaleX; destScaleY = other.destScaleY;
    angle = other.angle;
    
    effectVal = other.effectVal;
    
    lat = other.lat; lon = other.lon;
    
    borderColor = other.borderColor;
    destBColor = other.destBColor;
    baseBColor = other.baseBColor;
    
    name = other.name;
    siteID = other.siteID;
    nameStart = other.nameStart; nameEnd = other.nameEnd;
    finalName = other.finalName;
    
    font = other.font;
    
    borderTex = other.borderTex;
    twidth = other.twidth; theight = other.theight;
    
    selected = other.selected;
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
    angle = 0.0f;
    x = 0.0f; y = 0.0f; z = 0.0f;
    destX = x; destY = y;
    destScaleX = scaleX; destScaleY = scaleY;
    selected = false;
    
    destBColor.R = 1.0f; destBColor.G = 1.0f;
    destBColor.B = 1.0f; destBColor.A = 0.7f;
    borderColor = destBColor;
    baseBColor = destBColor;
    
    animated = true;
    finalName = false;
    nameStart = -1; nameEnd = -1;
    name = "";
    altName = "";
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

void RectangleBase::move( float _x, float _y )
{
    destX = _x;
    if ( !animated ) x = _x;
    destY = _y;
    if ( !animated ) y = _y;
}

void RectangleBase::setPos( float _x, float _y )
{
    destX = _x; x = _x;
    destY = _y; y = _y;
}

void RectangleBase::setScale( float xs, float ys )
{
    destScaleX = xs; destScaleY = ys;
    if ( !animated ) { scaleX = xs; scaleY = ys; }
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

void RectangleBase::setName( std::string s )
{
    name = s;
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
        return name.substr( nameStart, nameEnd-nameStart );
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

void RectangleBase::setSelect( bool select )
{
    selected = select;
    if ( select )
    {
        destBColor.R = 1.0f; destBColor.G = 1.0f;
        destBColor.B = 0.0f, destBColor.A = 0.8f;
    }
    else
    {
        destBColor = baseBColor;
    }
    
    if ( !animated )
        borderColor = destBColor;
}

void RectangleBase::setEffectVal( float f )
{
    effectVal = f;
}

void RectangleBase::setAnimation( bool anim )
{
    animated = anim;
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

bool RectangleBase::usingFinalName()
{
    return finalName;
}

bool RectangleBase::updateName()
{
    return false;
}

void RectangleBase::setSubstring( int start, int end )
{
    nameStart = start;
    nameEnd = end;
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

void RectangleBase::draw()
{
    animateValues();
    // note that the position should be set before calling this
    
    // set up our position
    glPushMatrix();

    glRotatef(angle, 0.0, 1.0, 0.0);
    glTranslatef(x,y,z);
    
    // draw the border first
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    //glBlendFunc( GL_ONE, GL_ONE );
    
    /*printf( "DRAWPRE\n" );
    GLenum  gl_error = glGetError();
    for (; (gl_error); gl_error = glGetError()) {
        fprintf(stderr, "%s\n", (const GLchar*)gluErrorString(gl_error));
    }*/
    
    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, borderTex );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 
        twidth);
    
    /*printf( "DRAWPOSTBIND\n" );
    gl_error = glGetError();
    for (; (gl_error); gl_error = glGetError()) {
        fprintf(stderr, "%s\n", (const GLchar*)gluErrorString(gl_error));
    }*/
    
    float s = (float)twidth / (float)GLUtil::getInstance()->pow2( twidth );
    float t = (float)theight / (float)GLUtil::getInstance()->pow2( theight );
    
    glScalef( 1.0f+effectVal, 1.0f+effectVal, 0.0f );
    
    // X & Y distances from center to edge
    float Xdist = getWidth()/2.0f + 0.3f;
    float Ydist = getHeight()/2.0f + 0.3f;
    
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
    
    /*printf( "DRAWPOSTCOORD\n" );
    gl_error = glGetError();
    for (; (gl_error); gl_error = glGetError()) {
        fprintf(stderr, "%s\n", (const GLchar*)gluErrorString(gl_error));
    }*/
    
    glEnd();
    glDisable( GL_BLEND );
    glDisable( GL_TEXTURE_2D );
    
    /*printf( "DRAWPOSTPOST\n" );
    gl_error = glGetError();
    for (; (gl_error); gl_error = glGetError()) {
        fprintf(stderr, "%s\n", (const GLchar*)gluErrorString(gl_error));
    }*/
    
    glPushMatrix();

    float yOffset = 0.4f;
    float scaleFactor = 0.006f * scaleX / 10.0f * 1.5f;
    if ( isGroup() ) 
    {
        yOffset = 0.9f;
        scaleFactor = 0.006f * scaleX / 10.0f;
    }
    
    glTranslatef( -getWidth()/2.0f, getHeight()/2.0f+yOffset, 0.0f );
    glRasterPos2f( -getWidth()/2.0f, getHeight()/2.0f+yOffset );
    glScalef( scaleFactor, scaleFactor, scaleFactor );
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    if ( font )
    {
        std::string renderedName = getSubName();
        const char* nc = renderedName.c_str();
        font->Render(nc);
    }
    
    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);
    
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
}

void RectangleBase::animateValues()
{
    // movement animation
    x += (destX-x)/7.5f;
    y += (destY-y)/7.5f;
    scaleX += (destScaleX-scaleX)/7.5f;
    scaleY += (destScaleY-scaleY)/7.5f;
    
    borderColor.R += (destBColor.R-borderColor.R)/3.0f;
    borderColor.G += (destBColor.G-borderColor.G)/3.0f;
    borderColor.B += (destBColor.B-borderColor.B)/3.0f;
    borderColor.A += (destBColor.A-borderColor.A)/3.0f;
    
    // snap to the destination, since we'll never actually get there via
    // the above lines due to roundoff errors
    if ( fabs(destX-x) < 0.01f ) x = destX;
    if ( fabs(destY-y) < 0.01f ) y = destY;
    if ( fabs(destScaleX-scaleX) < 0.01f ) scaleX = destScaleX;
    if ( fabs(destScaleY-scaleY) < 0.01f ) scaleY = destScaleY;
    if ( fabs(destBColor.R-borderColor.R) < 0.01f ) 
        borderColor.R = destBColor.R;
    if ( fabs(destBColor.G-borderColor.G) < 0.01f ) 
        borderColor.G = destBColor.G;
    if ( fabs(destBColor.B-borderColor.B) < 0.01f ) 
        borderColor.B = destBColor.B;
    if ( fabs(destBColor.A-borderColor.A) < 0.01f ) 
        borderColor.A = destBColor.A;
}
