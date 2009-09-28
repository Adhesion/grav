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
    baseBColor.R = random32()/random32_max();
    baseBColor.G = 1.0f;
    baseBColor.B = 0.0f;
    baseBColor.A = 0.85f;
    destBColor = baseBColor;
    name = std::string( "Group" );
    
    buffer = 1.0f;
}

Group::~Group()
{
    removeAll();
    printf( "group destructor\n" );
}

void Group::draw()
{
    animateValues();
    //printf( "drawing group at %f,%f\n", x, y );
    
    // set up our position
    glPushMatrix();

    glRotatef(angle, 0.0, 1.0, 0.0);
    glTranslatef(x,y,z);
    
    RectangleBase::draw();
    
    glPopMatrix();
    
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
}

void Group::remove( RectangleBase* object )
{
    object->setGroup( NULL );
    std::vector<RectangleBase*>::iterator i = objects.begin();
    while ( *i != object ) i++;
    objects.erase( i );
    if ( objects.size() > 0 )
        rearrange();
}

void Group::removeAll()
{
    for ( unsigned int i = 0; i < objects.size(); i++ )
    {
        remove( objects[i] );
    }
}

void Group::rearrange()
{
    float largestWidth = 0.0f, largestHeight = 0.0f; // for finding the biggest
                                                     // object in the group
                                                     
    for ( unsigned int i = 0; i < objects.size(); i++ )
    {
        largestWidth = std::max( largestWidth, objects[i]->getWidth() );
        largestHeight = std::max( largestHeight, objects[i]->getHeight() );
    }
    
    int numCol = ceil( sqrt( objects.size() ) );
    int numRow = objects.size() / numCol + ( objects.size() % numCol > 0 );
    
    float bufferSpaceX = (numCol-1) * buffer;
    float bufferSpaceY = (numRow-1) * buffer;
    printf( "rearranging %s...\n", name.c_str() );
    printf( "grid is %i x %i\n", numCol, numRow );
    printf( "largest object is %fx%f\n", largestWidth, largestHeight );
    
    float xOffset = ((numCol-1.0f) / 2.0f * largestWidth)
                    + (bufferSpaceX/2.0f);
    float yOffset = ((numRow-1.0f) / 2.0f * largestHeight)
                    + (bufferSpaceY/2.0f);
    
    float nextX = destX - xOffset;
    float nextY = destY + yOffset;
    
    printf( "group pos is %f,%f\n", x, y );
    printf( "starting video pos is %f,%f\n", nextX, nextY );
    printf( "offsets are %f,%f\n", xOffset, yOffset );
    printf( "offsets in relative units: %f,%f\n", (numCol-1.0f) / 2.0f,
                (numRow-1.0f) / 2.0f );
    
    for ( unsigned int i = 0; i < objects.size(); i++ )
    {
        printf( "moving object %i to %f,%f\n", i, nextX, nextY );
        objects[i]->move( nextX, nextY );
        nextX += largestWidth + bufferSpaceX;
        if ( (signed int)i%numCol == numCol-1 )
        {
            printf( "on %ith video, going to next row\n", i );
            nextX = destX - xOffset;
            nextY -= (largestHeight + bufferSpaceY);
        }
    }
    
    setScale(largestWidth * numCol + bufferSpaceX,
            largestHeight * numRow + bufferSpaceY);
}

void Group::move( float _x, float _y )
{
    printf( "moving group\n" );
    for ( unsigned int i = 0; i < objects.size(); i++ )
    {
        objects[i]->move( _x + objects[i]->getX() - x, 
                          _y + objects[i]->getY() - y );
    }
    RectangleBase::move( _x, _y );
}

void Group::setPos( float _x, float _y )
{
    for ( unsigned int i = 0; i < objects.size(); i++ )
    {
        objects[i]->setPos( _x + objects[i]->getX() - x, 
                            _y + objects[i]->getY() - y );
    }
    RectangleBase::setPos( _x, _y );
}
