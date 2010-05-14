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
    printf( "LayoutManager::perimeter: screen bounds: %f,%f %f,%f\n",
            screenL, screenR, screenU, screenD );

    float spaceAspect = (boundR-boundL) / (screenU-screenD);
    int topNum, sideNum, bottomNum;

    if ( objects.size() == 1 )
    {
        topNum = 1; sideNum = 0; bottomNum = 0;
    }
    else
    {
        topNum = floor( spaceAspect * (float)objects.size() / 2.0f );
        sideNum = ceil( (1.0f - spaceAspect) * (float)objects.size() / 2.0f );
        bottomNum = std::max( (int)objects.size() - topNum - (sideNum*2), 0 );
    }

    printf( "LayoutManager::perimeter: aspect of area: %f, %i %i\n",
            spaceAspect, topNum, sideNum );

    // create lists of objects on top,right,down,left areas and send them
    // to be arranged
    std::vector<RectangleBase*> topObjs, rightObjs, bottomObjs, leftObjs;
    int end = topNum;

    if ( topNum > 0 )
    {
        printf( "arranging objects %d to %d to top\n", 0, end-1 );
        for ( int i = 0; i < end; i++ )
            topObjs.push_back( objects[i] );
        // constant on top is for space for text
        gridArrange( boundL, boundR, screenU-0.8f, boundU, topNum, 1, true,
                     false, true, topObjs );
    }

    end = topNum + sideNum;

    if ( sideNum > 0 )
    {
        printf( "arranging objects %d to %d to right\n", topNum, end-1 );
        for ( int i = topNum; i < end; i++ )
            rightObjs.push_back( objects[i] );
        gridArrange( boundR, screenR, screenU, screenD, 1, sideNum, false, true,
                        true, rightObjs );
    }

    end = topNum + sideNum + bottomNum;

    if ( bottomNum > 0 )
    {
        printf( "arranging objects %d to %d to bottom\n", topNum + sideNum,
                end-1 );
        for ( int i = end-1; i >= topNum + sideNum; i-- )
            bottomObjs.push_back( objects[i] );
        gridArrange( boundL, boundR, boundD, screenD, bottomNum, 1, true,
                        false, true, bottomObjs );
    }

    if ( sideNum > 0 )
    {
        printf( "arranging objects %d to %d to left\n",
                topNum + sideNum + bottomNum, objects.size()-1 );
        for ( int i = objects.size()-1; i >= topNum + sideNum + bottomNum; i-- )
            leftObjs.push_back( objects[i] );
        gridArrange( screenL, boundL, screenU, screenD, 1, sideNum, false, true,
                        true, leftObjs );
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
