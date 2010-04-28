/**
 * @file LayoutManager.h
 * Definition of the LayoutManager, which takes objects and arranges them into
 * grid, perimeter, fullscreen, etc.
 * @author Andrew Ford
 */

#include <cstdio>
#include <cmath>

#include "LayoutManager.h"
#include "RectangleBase.h"

LayoutManager::LayoutManager()
{ }

void LayoutManager::perimeterArrange( RectangleBase screenRect,
                                        RectangleBase boundRect,
                                        std::vector<RectangleBase*> objects )
{
    float screenL = screenRect.getLBound();
    float screenR = screenRect.getRBound();
    float screenU = screenRect.getUBound();
    float screenD = screenRect.getDBound();
    
    float boundL = boundRect.getLBound();
    float boundR = boundRect.getRBound();
    float boundU = boundRect.getUBound();
    float boundD = boundRect.getDBound();
    
    perimeterArrange( screenL, screenR, screenU, screenD, boundL, boundR,
                        boundU, boundD, objects );
}

void LayoutManager::perimeterArrange( float screenL, float screenR,
                                        float screenU, float screenD,
                                        float boundL, float boundR,
                                        float boundU, float boundD,
                                        std::vector<RectangleBase*> objects )
{
    printf( "gravManager::perimeter: screen bounds: %f,%f %f,%f\n",
            screenL, screenR, screenU, screenD );
    
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
            // if we've gone over the side, change directions
            //if ( ( curX > (screenR-(nextObj->getWidth()/2.0f)) && Xdir == 1.0f ) ||
            //     ( curX < (screenL+(nextObj->getWidth()/2.0f)) && Xdir == -1.0f ) )
            if ( ( curX+obj->getWidth() > screenR && Xdir == 1.0f ) ||
                 ( curX-obj->getWidth() < screenL && Xdir == -1.0f ) )
            {
                // we're turning, so place the current object on the next area
                // and set the next object to go past that
                turnCount++;
                turnPoint = i;
                printf( "gravManager::perimeter: TURN X to Y\n" );
                Ydir *= -1.0f;
                if ( turnCount == 1 )
                {
                    curX = midRight;
                    curY = boundU - (buffer*2.0f) - nextObj->getHeight()/2.0f
                            - obj->getHeight();
                    numR++;
                }
                else if ( turnCount == 3 )
                {
                    curX = midLeft;
                    curY = boundD + (buffer*2.0f) + nextObj->getHeight()/2.0f
                            + obj->getHeight();
                    numL++;
                }
            }
            else
            {
                // otherwise this spot is valid, so increment the X direction
                curX += (nextObj->getWidth()/2.0f + obj->getWidth()/2.0f
                            + buffer) * Xdir;
                
                if ( turnCount == 0 )
                    numU++;
                else if ( turnCount == 2 )
                    numD++;
            }
        }
        else if ( turnCount % 2 == 1 )
        {
            // if we've gone over the side, change directions
            //if ( ( curY > (boundU-(nextObj->getHeight()/2.0f)) && Ydir == 1.0f ) ||
            //     ( curY < (boundD+(nextObj->getHeight()/2.0f)) && Ydir == -1.0f ) )
            if ( ( curY+obj->getHeight() > boundU && Ydir == 1.0f ) ||
                 ( curY-obj->getHeight() < boundD && Ydir == -1.0f ) )
            {
                turnCount++;
                turnPoint = i;
                printf( "gravManager::perimeter: TURN\n" );
                Xdir *= -1.0f;
                if ( turnCount == 2 )
                {
                    curX = midRight - (buffer*2.0f) - obj->getWidth();
                    curY = midDown;
                    numD++;
                }
                else if ( turnCount == 4 ) // meaning we've gone over?
                {
                    curX = midLeft + (buffer*2.0f) + obj->getWidth();
                    curY = midUp;
                    numU++;
                }
            }
            else
            {
                printf( "valid y preincrement is %f,%f\n", curX, curY );
                // this pos is valid so increment Y
                curY += (obj->getHeight()/2.0f + nextObj->getHeight()/2.0f + buffer)
                            * Ydir;
                printf( "on y, valid, incremented, at %f,%f\n", curX, curY );
                
                if ( turnCount == 1 )
                    numR++;
                else if ( turnCount == 3 )
                    numL++;
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
        float sizeAvg = 0.0f;
        float scaleAmt = -0.05f * (turnCount-2.0f);
        for ( unsigned int i = 0; i < numObjects; i++ )
        {
            sizeAvg += (objects[i]->getWidth()*objects[i]->getHeight());
        }
        sizeAvg /= numObjects;
        for ( unsigned int i = 0; i < numObjects; i++ )
        {
            RectangleBase* obj = objects[i];
            if ( obj->getWidth()*obj->getHeight() > sizeAvg )
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
        
        if ( numU > 0 )
        {
            printf( "arranging objects %d to %d to top\n", 0, numU-1 );
            for ( int i = 0; i < numU; i++ )
                topObjs.push_back( objects[i] );
            // constant on top is for space for text
            gridArrange( screenL, screenR, screenU-0.8f, boundU, numU, 1, true,
                         false, false, topObjs );
        }
        
        if ( numR > 0 )
        {
            printf( "arranging objects %d to %d to right\n", numU, numU+numR-1 );
            for ( int i = numU; i < numU+numR; i++ )
                rightObjs.push_back( objects[i] );
            gridArrange( boundR, screenR, boundU, boundD, 1, numR, false, true,
                            false, rightObjs );
        }
        
        if ( numD > 0 )
        {
            printf( "arranging objects %d to %d to bottom\n", numU+numR, numU+numR+numD-1 );
            for ( int i = numU+numR+numD-1; i >= numU+numR; i-- )
                bottomObjs.push_back( objects[i] );
            gridArrange( screenL, screenR, boundD, screenD, numD, 1, true,
                            false, false, bottomObjs );
        }
        
        if ( numL > 0 )
        {
            for ( int i = numObjects-1; i >= numU+numR+numD; i-- )
                leftObjs.push_back( objects[i] );
            gridArrange( screenL, boundL, boundU, boundD, 1, numL, false, true,
                            false, leftObjs );
        }
    }
}

bool LayoutManager::gridArrange( RectangleBase boundRect, int numX, int numY,
                                    bool horiz, bool edge, bool resize,
                                    std::vector<RectangleBase*> objects )
{
    float boundL = boundRect.getLBound();
    float boundR = boundRect.getRBound();
    float boundU = boundRect.getUBound();
    float boundD = boundRect.getDBound();
    
    return gridArrange( boundL, boundR, boundU, boundD, numX, numY, horiz, edge,
                            resize, objects );
}

bool LayoutManager::gridArrange( float boundL, float boundR, float boundU,
                                    float boundD, int numX, int numY,
                                    bool horiz, bool edge, bool resize,
                                    std::vector<RectangleBase*> objects )
{
    // if there's too many objects, fail
    if (  objects.size() > (unsigned int)(numX * numY) )
        return false;
    
    // if we only have one object, just fullscreen it to the area
    if ( objects.size() == 1 )
    {
        fullscreen( boundL, boundR, boundU, boundD, objects[0] );
        return true;
    }
    
    printf( "grid:bounds: %f,%f %f,%f\n", boundL, boundR, boundU, boundD );
    
    float span; // height of rows if going horizontally,
                // width of columns if going vertically
    float stride; // distance to move each time
    float curX, curY;
    float edgeL = boundL, edgeR = boundR, edgeU = boundU, edgeD = boundD;
    
    // set up span and stride, etc differently for horizontal vs vertical
    // arrangement
    if ( horiz )
    {
        span = (boundU-boundD) / numY;
        stride = (boundR-boundL) / numX;
        
        edgeL = boundL + 0.2f + (stride / 2);
        edgeR = boundR - 0.2f - (stride / 2);
        printf( "grid: edges are %f,%f\n", edgeL, edgeR );
        if ( edge ) stride = (edgeR-edgeL) / std::max(1, (numX-1));
        
        curY = boundU - (span/2.0f);
        
        if ( numX == 1 )
            curX = (boundR+boundL)/2.0f;
        else
        {
            if ( edge )
                curX = edgeL;
            else
                curX = boundL + (stride/2.0f);
        }
    }
    else
    {
        span = (boundR-boundL) / numX;
        stride = (boundU-boundD) / numY;
        
        edgeU = boundU - 0.2f - (stride / 2);
        edgeD = boundD + 0.2f + (stride / 2);
        if ( edge ) stride = (edgeU-edgeD) / std::max(1, (numY-1));

        curX = boundL + (span/2.0f);
        
        if ( numY == 1 )
            curY = (boundU+boundD)/2.0f;
        else
        {
            if ( edge )
                curY = edgeU;
            else
                curY = boundU - (stride/2.0f);
        }
    }
    
    printf( "grid: starting at %f,%f\n", curX, curY );
    printf( "grid: stride is %f\n", stride );
    printf( "grid: span is %f\n", span );
    
    // if we're resizing them, do it on a first pass so the calculations later
    // are correct
    if ( resize )
    {
        for ( unsigned int i = 0; i < objects.size(); i++ )
        {
            float objectAspect =
                        objects[i]->getDestWidth()/objects[i]->getDestHeight();
            float aspect = 1.0f;
            float newWidth = 1.0f;
            float newHeight = 1.0f;
            if ( horiz )
            {
                aspect = stride / span;
                newHeight = span * 0.85f;
                newWidth = stride * 0.85f;
            }
            else
            {
                aspect = span / stride;
                newHeight = stride * 0.85f;
                newWidth = span * 0.85f;
            }
            if ( aspect > objectAspect )
            {
                printf( "layout setting height to %f\n", newHeight );
                objects[i]->setHeight( newHeight );
            }
            else
            {
                printf( "layout setting width to %f\n", newWidth );
                objects[i]->setWidth( newWidth );
            }
        }
    }
    
    for ( unsigned int i = 0; i < objects.size(); i++ )
    {
        printf( "grid: moving object %i to %f,%f\n", i, curX, curY );
        objects[i]->move( curX, curY );
        int objectsLeft = (int)objects.size() - i - 1;
        
        if ( horiz )
        {
            curX += stride;
            if ( (i+1) % numX == 0 )
            {
                printf( "grid: changing to new row\n" );
                curY -= span;
                // if the number of objects we have left is less than a full
                // row/column, change stride such that it's evenly spaced
                if ( objectsLeft < numX )
                {
                    if ( edge )
                        stride = (edgeR-edgeL) / std::max(1, (objectsLeft-1));
                    else
                        stride = (boundR-boundL) / (objectsLeft);
                }
                curX = boundL + (stride/2.0f);
            }
        }
        else
        {
            curY -= stride;
            if ( (i+1) % numY == 0 )
            {
                curX += span;
                // if the number of objects we have left is less than a full
                // row/column, change stride such that it's evenly spaced
                if ( objectsLeft < numY )
                {
                    if ( edge )
                        stride = (edgeU-edgeD) / std::max(1, (objectsLeft-1));
                    else
                        stride = boundU-boundD / (objectsLeft+1);
                }
                curY = boundU - (stride/2.0f);
            }
        }
    }
    
    return true;
}

bool LayoutManager::fullscreen( RectangleBase boundRect, RectangleBase* object )
{
    float boundL = boundRect.getLBound();
    float boundR = boundRect.getRBound();
    float boundU = boundRect.getUBound();
    float boundD = boundRect.getDBound();
    
    return fullscreen( boundL, boundR, boundU, boundD, object );
}

bool LayoutManager::fullscreen( float boundL, float boundR, float boundU,
                                    float boundD, RectangleBase* object )
{
    float spaceAspect = fabs((boundR-boundL)/(boundU-boundD));
    float objectAspect = object->getWidth()/object->getHeight();
    
    if ( spaceAspect > objectAspect )
    {
        object->setHeight( boundU-boundD );
    }
    else
    {
        object->setWidth( boundR-boundL );
    }
    
    object->move( (boundR+boundL)/2.0f, (boundU+boundD)/2.0f );
    
    return true;
}
