/*
 * @file Group.cpp
 *
 * Implementation of the Group class, which can contain other RectangleBases.
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

#include "Group.h"
#include <VPMedia/random_helper.h>
#include <cmath>
#include <sstream>

Group::Group( float _x, float _y ) :
    RectangleBase( _x, _y )
{
    baseBColor.R = (float)random32()/(float)random32_max();
    baseBColor.G = (float)random32()/(float)random32_max();
    baseBColor.B = (float)random32()/(float)random32_max();
    baseBColor.A = 0.7f;
    destBColor = baseBColor;
    setColor( destBColor );
    name = std::string( "Group" );

    locked = true;
    showLockStatus = true;
    preserveChildAspect = true;

    rearrangeStyle = ONECOLUMN;

    buffer = 1.0f;
}

Group::~Group()
{
    removeAll();
}

void Group::draw()
{
    RectangleBase::draw();

    for ( unsigned int i = 0; i < objects.size(); i++ )
    {
        objects[i]->draw();
    }
}

void Group::add( RectangleBase* object )
{
    objects.push_back( object );
    object->setGroup( this );

    rearrange();

    for ( unsigned int i = 0; i < objects.size(); i++ )
        objects[i]->updateName();

    updateName();
}

void Group::remove( RectangleBase* object, bool move )
{
    if ( object == NULL )
    {
        gravUtil::logError( "Group::remove: NULL object input\n" );
        return;
    }

    std::vector<RectangleBase*>::iterator i = objects.begin();
    while ( i != objects.end() && *i != object ) i++;
    if ( i == objects.end() )
    {
        gravUtil::logError( "Group::remove: object %s not found\n",
            object->getName().c_str() );
        return;
    }

    object->setGroup( NULL );
    object->setSubstring( -1, -1 );

    objects.erase( i );
    if ( objects.size() > 0 && move )
        rearrange();
}

std::vector<RectangleBase*>::iterator Group::remove(
                            std::vector<RectangleBase*>::iterator i, bool move )
{
    if ( i == objects.end() || (*i) == NULL )
    {
        gravUtil::logError( "Group::remove: invalid object input\n" );
        return objects.end();
    }

    (*i)->setGroup( NULL );
    (*i)->setSubstring( -1, -1 );

    std::vector<RectangleBase*>::iterator ret = objects.erase( i );
    if ( objects.size() > 0 && move )
        rearrange();
    return ret;
}

std::vector<RectangleBase*>::iterator Group::getBeginIterator()
{
    return objects.begin();
}

std::vector<RectangleBase*>::iterator Group::getEndIterator()
{
    return objects.end();
}

void Group::removeAll()
{
    std::vector<RectangleBase*>::iterator i = objects.begin();
    while ( i != objects.end() )
    {
        i = remove( i, false );
    }
}

RectangleBase* Group::operator[]( int i )
{
    return objects[i];
}

int Group::numObjects()
{
    return objects.size();
}

bool Group::isGroup()
{
    return true;
}

void Group::rearrange()
{
    rearrange( objects );
}

void Group::rearrange( std::vector<RectangleBase*> inObjs )
{
    // it doesn't make sense to rearrange 0 objects, plus having objects.size
    // = 0 will cause div by 0 crashes later
    if ( inObjs.size() == 0 ) return;

    std::map<std::string, std::vector<RectangleBase*> > data;
    data["objects"] = inObjs;

    std::map<std::string, std::string> opts;
    opts["preserveAspect"] =
            std::string( preserveChildAspect ? "True" : "False" );

    std::ostringstream ss;

    switch ( rearrangeStyle )
    {
    case ASPECT:
    {
        // for finding the biggest object in the group
        float largestWidth = 0.0f, largestHeight = 0.0f;

        for ( unsigned int i = 0; i < inObjs.size(); i++ )
        {
            largestWidth = std::max( largestWidth, inObjs[i]->getWidth() );
            largestHeight = std::max( largestHeight, inObjs[i]->getHeight() );
        }

        int numCol = ceil( sqrt( inObjs.size() ) );
        int numRow = inObjs.size() / numCol + ( inObjs.size() % numCol > 0 );

        // resize the group based on the aspect ratios of the current member(s)
        if ( inObjs.size() == 1 )
        {
            float objAspect =
                    inObjs[0]->getDestWidth() / inObjs[0]->getDestHeight();
            float diff = objAspect / ( getDestWidth() / getDestHeight() );
            RectangleBase::setScale( destScaleX * diff, destScaleY );
        }
        else
        {
            float aspect = getDestWidth() / getDestHeight();
            float newAspect = (numCol*1.33f) / numRow;
            if ( newAspect > aspect )
                RectangleBase::setScale( destScaleX * (newAspect/aspect),
                                            destScaleY );
            else
                RectangleBase::setScale( destScaleX,
                                            destScaleY * (aspect/newAspect) );
        }

        ss << numCol;
        opts["numX"] = ss.str();
        ss.str( "" );

        ss << numRow;
        opts["numY"] = ss.str();
        ss.str( "" );

        break;
    }

    case ONEROW:
    {
        ss << inObjs.size();
        opts["numX"] = ss.str();
        ss.str( "" );
        opts["numY"] = "1";
        break;
    }

    case ONECOLUMN:
    {
        ss << inObjs.size();
        opts["numX"] = "1";
        opts["numY"] = ss.str();
        ss.str( "" );
        opts["horiz"] = "False";
        break;
    }

    default:
        break;
    }

    layouts.arrange( "grid",
                     getDestLBound(), getDestRBound(),
                     getDestUBound(), getDestDBound(),
                     0, 0, 0, 0,
                     data, opts );
}

ArrangeStyle Group::getRearrange()
{
    return rearrangeStyle;
}

void Group::setRearrange( ArrangeStyle style )
{
    rearrangeStyle = style;
}

bool Group::updateName()
{
    if ( objects.size() == 0 ) return false;

    bool membersFinalized = true;
    bool nameChanged = false;
    std::string oldName = name;
    int shortestLength = 256;
    for ( unsigned int i = 0; i < objects.size(); i++ )
    {
        membersFinalized = membersFinalized && objects[i]->usingFinalName();
        shortestLength = std::min( shortestLength,
                                    (int)objects[i]->getName().length() );
    }

    // only try to create the name if all the members have their correct
    // names (ie, NAME vs CNAME)
    if ( !membersFinalized ) return false;

    int splitPos = 0;
    int startParen = -1;
    int endParen = -1;

    if ( objects.size() > 1 )
    {
        // loop through the characters and find the position of the end of the
        // first common substring
        bool equal = true;
        while ( equal && splitPos < shortestLength )
        {
            for ( unsigned int j = 1; j < objects.size(); j++ )
            {
                equal = equal && (objects[j-1]->getName().at(splitPos) ==
                                    objects[j]->getName().at(splitPos) );
            }
            splitPos++;
        }
        splitPos--;

        // if there are parentheses, split the strings based on the closest
        // left paren to the left of the difference position (if found)
        startParen = objects[0]->getName().rfind( '(', splitPos );
        endParen = objects[0]->getName().find( ')', splitPos );
        if ( endParen != -1 && startParen != -1 )
            splitPos = startParen;
    }
    else
    {
        // if there's only one, just split based on the rightmost & outermost
        // matched parens
        int i = objects[0]->getName().length()-1;
        int balance = 0;
        while ( i >= 0 && (balance != 0 || endParen == -1) )
        {
            char c = objects[0]->getName().at(i);
            if ( c == ')' )
            {
                if ( i == (int)objects[0]->getName().length()-1 )
                    endParen = i;
                balance++;
            }
            else if ( c == '(' )
            {
                startParen = i;
                balance--;
            }
            i--;
        }

        if ( endParen != -1 && startParen != -1 )
            splitPos = startParen;
    }

    // set the group's name to the common substring, and the members' names
    // to the remainder
    if ( splitPos > 0 )
        name = objects[0]->getName().substr(0, splitPos);
    else
        name = objects[0]->getName();
    nameChanged = name.compare( oldName ) != 0;

    for ( unsigned int k = 0; k < objects.size(); k++ )
    {
        // if we have more than one object in the group, re-find the positions
        // of the parentheses since the text inside them may not be the same
        // length for all objects
        if ( objects.size() > 1 )
        {
            startParen = objects[k]->getName().rfind( '(', splitPos );
            endParen = objects[k]->getName().find( ')', splitPos );
        }

        if ( endParen != -1 && startParen != -1 )
        {
            objects[k]->setSubstring( startParen+1, endParen );
        }
        else
        {
            objects[k]->setSubstring( splitPos,
                                objects[k]->getName().length() );
        }
    }

    if ( nameChanged )
        updateTextBounds();
    finalName = true;
    return nameChanged;
}

void Group::move( float _x, float _y )
{
    for ( unsigned int i = 0; i < objects.size(); i++ )
    {
        objects[i]->move( _x + objects[i]->getDestX() - destX,
                          _y + objects[i]->getDestY() - destY );
    }
    RectangleBase::move( _x, _y );
}

void Group::setPos( float _x, float _y )
{
    for ( unsigned int i = 0; i < objects.size(); i++ )
    {
        objects[i]->setPos( _x + objects[i]->getDestX() - destX,
                            _y + objects[i]->getDestY() - destY );
    }
    RectangleBase::setPos( _x, _y );
}

void Group::setScale( float xs, float ys )
{
    if ( locked )
        setScale( xs, ys, true );
    else
        setScale( xs, ys, false );
}

void Group::setScale( float xs, float ys, bool resizeMembers )
{
    RectangleBase::setScale( xs, ys );
    if ( resizeMembers )
    {
        rearrange();
        /*float Xratio = xs / destScaleX;
        float Yratio = ys / destScaleY;
        float min = std::min( Xratio, Yratio );

        for ( unsigned int i = 0; i < objects.size(); i++ )
        {
            float objScaleX = (objects[i]->getScaleX() * min);
            float objScaleY = (objects[i]->getScaleY() * min);
            float Xdist = objects[i]->getDestX() - destX;
            float Ydist = objects[i]->getDestY() - destY;
            Xdist *= Xratio;
            Ydist *= Yratio;

            // if the object scale values are close to the group's scale values,
            // then make it a bit smaller
            if ( (objScaleX >= (xs-0.5f)) || (objScaleY >= (ys-0.5f)) )
            {
                objScaleX -= 0.3f; objScaleY -= 0.3f;
            }
            objects[i]->setScale( objScaleX, objScaleY );
            objects[i]->move( destX+Xdist, destY+Ydist );
        }*/
    }
}

void Group::show( bool s, bool instant )
{
    RectangleBase::show( s, instant );
    for ( unsigned int i = 0; i < objects.size(); i++ )
    {
        objects[i]->show( s, instant );
    }
}
