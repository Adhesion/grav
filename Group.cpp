/*
 * @file Group.h
 * Implementation of the Group class.
 * @author Andrew Ford
 */

#include "Group.h"
#include <VPMedia/random_helper.h>
#include <cmath>

Group::Group( float _x, float _y ) :
    RectangleBase( _x, _y )
{
    baseBColor.R = (float)random32()/(float)random32_max();
    baseBColor.G = (float)random32()/(float)random32_max();
    baseBColor.B = (float)random32()/(float)random32_max();
    baseBColor.A = 0.7f;
    destBColor = baseBColor;
    name = std::string( "Group" );
    
    locked = true;
    showLockStatus = true;
    
    buffer = 1.0f;
}

Group::~Group()
{
    removeAll();
}

void Group::draw()
{
    //animateValues();
    //printf( "drawing group at %f,%f\n", x, y );
    
    RectangleBase::draw();
    
    //printf( "drawing %i objects in group\n", objects.size() );
    for ( unsigned int i = 0; i < objects.size(); i++ )
    {
        objects[i]->draw();
    }
}

void Group::add( RectangleBase* object )
{
    objects.push_back( object );
    object->setGroup( this );
    printf( "added %s to group %s\n", object->getName().c_str(),
                                        getName().c_str() );
    
    printf( "now rearranging %i objects\n", objects.size() );
    rearrange();
    
    for ( unsigned int i = 0; i < objects.size(); i++ )
        objects[i]->updateName();
    
    updateName();
}

void Group::remove( RectangleBase* object, bool move )
{
    //printf( "removing %s from group %s\n", object->getName().c_str(),
    //                                    getName().c_str() );
    object->setGroup( NULL );
    object->setSubstring( -1, -1 );

    std::vector<RectangleBase*>::iterator i = objects.begin();
    while ( *i != object ) i++;
    objects.erase( i );
    if ( objects.size() > 0 && move )
        rearrange();
}

void Group::removeAll()
{
    for ( unsigned int i = 0; i < objects.size(); )
    {
        remove( objects[i], false );
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
    // it doesn't make sense to rearrange 0 objects, plus having objects.size
    // = 0 will cause div by 0 crashes later
    if ( objects.size() == 0 ) return;
    
    float largestWidth = 0.0f, largestHeight = 0.0f; // for finding the biggest
                                                     // object in the group

    for ( unsigned int i = 0; i < objects.size(); i++ )
    {
        largestWidth = std::max( largestWidth, objects[i]->getWidth() );
        largestHeight = std::max( largestHeight, objects[i]->getHeight() );
    }
    
    int numCol = ceil( sqrt( objects.size() ) );
    int numRow = objects.size() / numCol + ( objects.size() % numCol > 0 );
    //printf( "Group:: group %s (%fx%f, %f,%f) rearranging\n", name.c_str(),
    //            getWidth(), getHeight(), destX, destY );

    // resize the group based on the aspect ratios of the current member(s)
    if ( objects.size() == 1 )
    {
        float objAspect =
                objects[0]->getDestWidth() / objects[0]->getDestHeight();
        float diff = objAspect / ( getDestWidth() / getDestHeight() );
        //printf( "Group::rearrange: modifying aspect by %f\n", diff );
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
    
    layouts.gridArrange( getLBound(), getRBound(), getUBound(), getDBound(),
                            true, false, true, objects, numCol, numRow );
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
    
    //printf( "group members names finalized, finding common string...\n" );
    
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
        //printf( ">1 startParen: %i, endParen: %i\n", startParen, endParen );
        if ( endParen != -1 && startParen != -1 )
            splitPos = startParen;
    }
    else
    {
        // if there's only one, just split based on the rightmost & outermost
        // matched parens
        unsigned int i = objects[0]->getName().length()-1;
        //printf( "name is %s, length is %i, i is %i\n",
        //        objects[0]->getName().c_str(), i+1, i );
        int balance = 0;
        while ( i >= 0 && (balance != 0 || endParen == -1) )
        {
            char c = objects[0]->getName().at(i);
            if ( c == ')' )
            {
                if ( i == objects[0]->getName().length()-1 )
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

        //printf( "=1 startParen: %i, endParen: %i\n", startParen, endParen );
        if ( endParen != -1 && startParen != -1 )
            splitPos = startParen;
    }
    
    // set the group's name to the common substring, and the members' names
    // to the remainder
    name = objects[0]->getName().substr(0, splitPos);
    nameChanged = name.compare( oldName ) == 0;
    
    //printf( "common substr is %s, up to pos %i\n", name.c_str(), splitPos );
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
        //printf( "object name (remainder) set to %s\n",
        //                    objects[k]->getSubName().c_str() );
    }
    
    //cutoffPos = -1;
    updateTextBounds( nameChanged );
    finalName = true;
    return nameChanged;
}

void Group::move( float _x, float _y )
{
    //printf( "moving group\n" );
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
        printf( "Group::setScale: scaling group %s to %f,%f, ratio is %f/%f\n",
                    name.c_str(), xs, ys, Xratio, Yratio );
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
