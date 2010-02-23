/**
 * @file LayoutManager.h
 * Definition of the LayoutManager, which takes objects and arranges them into
 * grid, perimeter, fullscreen, etc.
 * @author Andrew Ford
 */

#include <iostream>
#include <cmath>

#include "LayoutManager.h"
#include "RectangleBase.h"

LayoutManager::LayoutManager()
{ }

void LayoutManager::perimeterArrange( float screenL, float screenR,
                        float screenU, float screenD,
                        float boundL, float boundR, float boundU, float boundD,
                        std::vector<RectangleBase*> objects )
{
    // TODO: replace this with arguments - should be able to call on an
    //       arbitrary area
    /*GLUtil* glUtil = GLUtil::getInstance();
    GLdouble screenL, screenR, screenU, screenD, screenZ; // screenZ is unused
    glUtil->screenToWorld( (GLdouble)windowWidth, (GLdouble)windowHeight,
                          0.990991f, &screenR, &screenU, &screenZ );
    glUtil->screenToWorld( (GLdouble)0.0f, (GLdouble)0.0f,
                          0.990991f, &screenL, &screenD, &screenZ );
    printf( "gravManager::perimeter: screen bounds: %f,%f %f,%f\n",
            screenL, screenR, screenU, screenD );*/
    
    float midLeft = screenL + ( fabs( boundL - screenL ) / 2.0f );
    float midUp = screenU - ( fabs( screenU - boundU ) / 2.0f );
    float midRight = screenR - ( fabs( screenR - boundR ) / 2.0f );
    float midDown = screenD + ( fabs( boundD - screenD ) / 2.0f );
    float startX = midLeft;
    float startY = midUp;
    float maxX = std::min( fabs( screenR - boundR ), fabs( boundL - screenL ) );
    float maxY = std::min( fabs( screenU - boundU ), fabs( boundD - screenD ) )
                    - 0.8f; // extra constant is for text
    
    printf( "gravManager::perimeter: start point: %f,%f\n", startX, startY );
    printf( "gravManager::perimeter: maximums: %f,%f\n", maxX, maxY );
    
    unsigned int numObjects = objects.size();
    float buffer = 1.0f;
    float Xdir = 1.0f; float Ydir = 1.0f;
    float curX = startX; float curY = startY;
    int turnCount = 0;
    int numU = 0; int numR = 0; int numD = 0; int numL = 0;
    int* Xvals = new int[numObjects];
    int* Yvals = new int[numObjects];
    unsigned int turnPoint = 0;
    
    // do a dry run and find how the objects can fit into the space
    for ( unsigned int i = 0; i < numObjects; i++ )
    {
        RectangleBase* obj = objects[i];
        RectangleBase* nextObj;
        if ( i == numObjects-1 )
            nextObj = obj;
        else
            nextObj = objects[i+1];
        printf( "gravManager::perimeter: analyzing at %f,%f\n", curX, curY );
        Xvals[i] = curX; Yvals[i] = curY;
        
        // scale it down if it's bigger than the maximum X or Y sizes
        if ( obj->getWidth() > maxX )
            obj->setWidth( maxX );
        if ( obj->getHeight() > maxY )
            obj->setHeight( maxY );
        
        if ( turnCount % 2 == 0 )
        {
            // increment the X direction
            curX += (nextObj->getWidth()/2.0f + obj->getWidth()/2.0f + buffer)
                        * Xdir;
            
            if ( turnCount == 0 )
                numU++;
            else if ( turnCount == 2 )
                numD++;
            
            // if we've gone over the side, change directions
            //if ( ( curX > (screenR-(nextObj->getWidth()/2.0f)) && Xdir == 1.0f ) ||
            //     ( curX < (screenL+(nextObj->getWidth()/2.0f)) && Xdir == -1.0f ) )
            if ( ( curX > midRight && Xdir == 1.0f ) ||
                 ( curX < midLeft && Xdir == -1.0f ) )
            {
                turnCount++;
                turnPoint = i;
                printf( "gravManager::perimeter: TURN\n" );
                Ydir *= -1.0f;
                if ( turnCount == 1 )
                {
                    curX = midRight;
                    curY = boundU - buffer - nextObj->getHeight()/2.0f;
                }
                else if ( turnCount == 3 )
                {
                    curX = midLeft;
                    curY = boundD + buffer + nextObj->getHeight()/2.0f;
                }
            }
        }
        else if ( turnCount % 2 == 1 )
        {
            // increment Y
            curY += (obj->getHeight()/2.0f + nextObj->getHeight()/2.0f + buffer)
                        * Ydir;
            
            if ( turnCount == 1 )
                numR++;
            else if ( turnCount == 3 )
                numL++;
            
            float bottomThresh = boundD+(nextObj->getHeight()/2.0f);
            printf( "checking Y: bottom threshold is %f\n", bottomThresh );
            // if we've gone over the side, change directions
            //if ( ( curY > (boundU-(nextObj->getHeight()/2.0f)) && Ydir == 1.0f ) ||
            //     ( curY < (boundD+(nextObj->getHeight()/2.0f)) && Ydir == -1.0f ) )
            if ( ( curY > midUp && Ydir == 1.0f ) ||
                 ( curY < bottomThresh && Ydir == -1.0f ) )
            {
                turnCount++;
                turnPoint = i;
                printf( "gravManager::perimeter: TURN\n" );
                Xdir *= -1.0f;
                if ( turnCount == 2 )
                {
                    curX = midRight;
                    curY = midDown;
                }
                else if ( turnCount == 4 ) // meaning we've gone over?
                {
                    curX = midLeft;
                    curY = midUp;
                }
            }
        }
    }
    
    printf( "gravManager::perimeter: analysis done, counts URDL: %d,%d,%d,%d\n",
            numU, numR, numD, numL );
    
    // if we made more than 3 turns then there are overlapping videos so
    // make them all a bit smaller
    if ( turnCount > 3 && turnPoint != numObjects-1 ) 
                        // AND, there was at least one more video to analyze
    {
        float scaleAmt = -0.05f * (turnCount-2.0f);
        for ( unsigned int i = 0; i < numObjects; i++ )
        {
            RectangleBase* obj = objects[i];
            obj->setScale( obj->getScaleX()+obj->getScaleX()*scaleAmt,
                           obj->getScaleY()+obj->getScaleY()*scaleAmt );
        }
    }
    // otherwise we can move the videos to their positions
    else
    {     
        // create lists of objects on top,right,down,left areas and send them
        // to be arranged
        std::vector<RectangleBase*> topObjs, rightObjs, bottomObjs, leftObjs;
        
        printf( "arranging objects %d to %d to top\n", 0, numU-1 );
        if ( numU > 0 )
        {
            for ( int i = 0; i < numU; i++ )
                topObjs.push_back( objects[i] );
            gridArrange( screenL, screenR, screenU-0.8f, boundU, numU, 1, true,
                            topObjs ); // constant on top is for space for text
        }
        
        printf( "arranging objects %d to %d to right\n", numU, numU+numR-1 );
        if ( numR > 0 )
        {
            for ( int i = numU; i < numU+numR; i++ )
                rightObjs.push_back( objects[i] );
            gridArrange( boundR, screenR, boundU, boundD, 1, numR, false,
                            rightObjs );
        }
        
        printf( "arranging objects %d to %d to bottom\n", numU+numR, numU+numR+numD-1 );
        if ( numD > 0 )
        {
            for ( int i = numU+numR+numD-1; i >= numU+numR; i-- )
                bottomObjs.push_back( objects[i] );
            gridArrange( screenL, screenR, boundD, screenD, numD, 1, true,
                            bottomObjs );
        }
        
        if ( numL > 0 )
        {
            for ( int i = numObjects-1; i >= numU+numR+numD; i-- )
                leftObjs.push_back( objects[i] );
            gridArrange( screenL, boundL, boundU, boundD, 1, numL, false,
                            leftObjs );
        }
    }
}

bool LayoutManager::gridArrange( float boundL, float boundR, float boundU,
                                float boundD, int numX, int numY, bool horiz,
                                std::vector<RectangleBase*> objects )
{
    // if there's too many objects, fail
    if ( (unsigned int)(numX * numY) > objects.size() )
        return false;
    
    // TODO: potentially add resizing of videos if they're bigger than span
    // or stride    
    
    float span; // height of rows if going horizontally,
                // width of columns if going vertically
    float stride; // distance to move each time
    float curX, curY;
    if ( horiz )
    {
        span = (boundU-boundD)/numY;
        stride = (boundR-boundL)/(numX+1);
        curY = boundU - (span/2.0f);
        curX = boundL + stride;
    }
    else
    {
        span = (boundR-boundL)/numX;
        stride = (boundU-boundD)/(numY+1);
        curY = boundU - stride;
        curX = boundL + (span/2.0f);
    }
    
    for ( unsigned int i = 0; i < objects.size(); i++ )
    {
        objects[i]->move( curX, curY );
        
        if ( horiz )
        {
            curX += stride;
            if ( i+1 % numX == 0 )
            {
                curY += span;
                curX = boundL + stride;
            }
        }
        else
        {
            curY -= stride;
            if ( i+1 % numY == 0 )
            {
                curX += span;
                curY = boundU + stride;
            }
        }
    }
    
    return true;
}
