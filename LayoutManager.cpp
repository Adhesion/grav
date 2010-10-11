/**
 * @file LayoutManager.h
 * Definition of the LayoutManager, which takes objects and arranges them into
 * grid, perimeter, fullscreen, etc.
 * @author Andrew Ford
 *         Ralph Bean
 */

#include <cstdio>
#include <cmath>

#include "LayoutManager.h"
#include "RectangleBase.h"

LayoutManager::LayoutManager()
{ }


void LayoutManager::arrange( std::string method,
                             RectangleBase screenRect,
                             RectangleBase boundRect,
                             std::vector<RectangleBase*> objects,
                             std::map<std::string, std::string> options)
{
    float screenL = screenRect.getLBound();
    float screenR = screenRect.getRBound();
    float screenU = screenRect.getUBound();
    float screenD = screenRect.getDBound();

    float boundL = boundRect.getLBound();
    float boundR = boundRect.getRBound();
    float boundU = boundRect.getUBound();
    float boundD = boundRect.getDBound();

    arrange(method,
            screenL, screenR, screenU, screenD,
            boundL, boundR, boundU, boundD,
            objects, options);
}

void LayoutManager::arrange( std::string method,
                             float screenL, float screenR,
                             float screenU, float screenD,
                             float boundL, float boundR,
                             float boundU, float boundD,
                             std::vector<RectangleBase*> objects,
                             std::map<std::string, std::string> options)
{
    perimeterArrange(screenL, screenR, screenU, screenD,
                     boundL, boundR, boundU, boundD,
                     objects); // not passing options for now...
}


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
    //printf( "LayoutManager::perimeter: screen bounds: %f,%f %f,%f\n",
    //        screenL, screenR, screenU, screenD );

    float topRatio = (boundR-boundL) / ((screenU-screenD)+(boundR-boundL));
    float sideRatio = (screenU-screenD) / ((screenU-screenD)+(boundR-boundL));
    int topNum, sideNum, bottomNum;

    if ( objects.size() == 1 )
    {
        topNum = 1; sideNum = 0; bottomNum = 0;
    }
    else
    {
        topNum = floor( topRatio * (float)objects.size() / 2.0f );
        sideNum = ceil( sideRatio * (float)objects.size() / 2.0f );
        bottomNum = std::max( (int)objects.size() - topNum - (sideNum*2), 0 );
    }

    //printf( "LayoutManager::perimeter: ratios of area: %f %f, %i %i\n",
    //        topRatio, sideRatio, topNum, sideNum );

    // create lists of objects on top,right,down,left areas and send them
    // to be arranged
    std::vector<RectangleBase*> topObjs, rightObjs, bottomObjs, leftObjs;
    int end = topNum;

    if ( topNum > 0 )
    {
        //printf( "arranging objects %d to %d to top\n", 0, end-1 );
        for ( int i = 0; i < end; i++ )
            topObjs.push_back( objects[i] );
        // constant on top is for space for text
        gridArrange( boundL, boundR, screenU-0.8f, boundU, true, false, true,
                     topObjs, topNum, 1 );
    }

    end = topNum + sideNum;

    if ( sideNum > 0 )
    {
        //printf( "arranging objects %d to %d to right\n", topNum, end-1 );
        for ( int i = topNum; i < end; i++ )
            rightObjs.push_back( objects[i] );
        gridArrange( boundR, screenR, screenU, screenD, false, true, true,
                     rightObjs, 1, sideNum );
    }

    end = topNum + sideNum + bottomNum;

    if ( bottomNum > 0 )
    {
        //printf( "arranging objects %d to %d to bottom\n", topNum + sideNum,
        //        end-1 );
        for ( int i = end-1; i >= topNum + sideNum; i-- )
            bottomObjs.push_back( objects[i] );
        gridArrange( boundL, boundR, boundD, screenD, true, false, true,
                     bottomObjs, bottomNum, 1 );
    }

    if ( sideNum > 0 )
    {
        //printf( "arranging objects %d to %d to left\n",
        //        topNum + sideNum + bottomNum, objects.size()-1 );
        for ( int i = objects.size()-1; i >= topNum + sideNum + bottomNum; i-- )
            leftObjs.push_back( objects[i] );
        gridArrange( screenL, boundL, screenU, screenD, false, true, true,
                     leftObjs, 1, sideNum );
    }
}

bool LayoutManager::gridArrange( RectangleBase boundRect,
                                    bool horiz, bool edge, bool resize,
                                    std::vector<RectangleBase*> objects,
                                    int numX, int numY )
{
    float boundL = boundRect.getLBound();
    float boundR = boundRect.getRBound();
    float boundU = boundRect.getUBound();
    float boundD = boundRect.getDBound();
    
    return gridArrange( boundL, boundR, boundU, boundD, horiz, edge,
                            resize, objects, numX, numY );
}

bool LayoutManager::gridArrange( float boundL, float boundR, float boundU,
                                    float boundD,
                                    bool horiz, bool edge, bool resize,
                                    std::vector<RectangleBase*> objects,
                                    int numX, int numY )
{
    if ( objects.size() == 0 )
        return false;

    // both of these being 0 (also the default vals) means we should figure out
    // the proper numbers here
    if ( numX == 0 && numY == 0 )
    {
        numX = ceil( sqrt( objects.size() ) );
        numY = objects.size() / numX + ( objects.size() % numX > 0 );
        //printf( "layout: doing grid arrangement with %i objects (%ix%i)\n",
        //            objects.size(), numX, numY );
    }

    // if there's too many objects, fail
    if (  objects.size() > (unsigned int)(numX * numY) )
        return false;

    // if we only have one object, just fullscreen it to the area
    if ( objects.size() == 1 )
    {
        fullscreen( boundL, boundR, boundU, boundD, objects[0] );
        return true;
    }
    
    //printf( "grid:bounds: %f,%f %f,%f\n", boundL, boundR, boundU, boundD );
    
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
        //printf( "grid: edges are %f,%f\n", edgeL, edgeR );
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
    
    //printf( "grid: starting at %f,%f\n", curX, curY );
    //printf( "grid: stride is %f\n", stride );
    //printf( "grid: span is %f\n", span );
    
    // if we're resizing them, do it on a first pass so the calculations later
    // are correct
    if ( resize )
    {
        for ( unsigned int i = 0; i < objects.size(); i++ )
        {
            float objectAspect =
                       objects[i]->getTotalWidth()/objects[i]->getTotalHeight();
            float aspect = 1.0f;
            float newWidth = 1.0f;
            float newHeight = 1.0f;
            if ( horiz )
            {
                aspect = stride / span;
                // the .95s are to push things away from the edges, which can
                // cut close due to roundoff error etc.
                newHeight = span * 0.95f;
                newWidth = stride * 0.95f;
            }
            else
            {
                aspect = span / stride;
                newHeight = stride * 0.95f;
                newWidth = span * 0.95f;
            }
            if ( aspect > objectAspect )
            {
                //printf( "layout setting height to %f\n", newHeight );
                objects[i]->setTotalHeight( newHeight );
            }
            else
            {
                //printf( "layout setting width to %f\n", newWidth );
                objects[i]->setTotalWidth( newWidth );
            }
        }
    }
    
    for ( unsigned int i = 0; i < objects.size(); i++ )
    {
        //printf( "grid: moving object %i to %f,%f\n", i, curX, curY );
        objects[i]->move( curX, curY - objects[i]->getCenterOffsetY() );
        int objectsLeft = (int)objects.size() - i - 1;
        
        if ( horiz )
        {
            curX += stride;
            if ( (i+1) % numX == 0 )
            {
                //printf( "grid: changing to new row\n" );
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
    float objectAspect = object->getTotalWidth()/object->getTotalHeight();
    //printf( "LayoutManager::fullscreen: aspects are %f in %f\n", objectAspect, spaceAspect );

    if ( ( spaceAspect - objectAspect ) > 0.01f )
    {
        //printf( "LayoutManager::fullscreen: setting height to %f\n",
        //        boundU-boundD );
        object->setTotalHeight( boundU-boundD );
    }
    else
    {
        //printf( "LayoutManager::fullscreen: setting width to %f\n",
        //        boundR-boundL );
        object->setTotalWidth( boundR-boundL );
    }

    // TODO need to change this if getCenterOffsetX() is ever meaningful
    object->move( (boundR+boundL)/2.0f, ((boundU+boundD)/2.0f) -
                    object->getCenterOffsetY() );

    return true;
}

bool LayoutManager::focus( RectangleBase boundRect,
                std::vector<RectangleBase*> outers,
                std::vector<RectangleBase*> inners,
                float scaleX, float scaleY )
{
    float boundL = boundRect.getLBound();
    float boundR = boundRect.getRBound();
    float boundU = boundRect.getUBound();
    float boundD = boundRect.getDBound();

    //printf( "LayoutManager::focusing, outer rect %f %f %f %f\n", boundL, boundR,
    //        boundU, boundD );

    return focus( boundL, boundR, boundU, boundD, outers, inners,
                        scaleX, scaleY );
}

bool LayoutManager::focus( float boundL, float boundR, float boundU,
                float boundD, std::vector<RectangleBase*> outers,
                std::vector<RectangleBase*> inners,
                float scaleX, float scaleY )
{
    float centerX = ( boundL + boundR ) / 2.0f;
    float centerY = ( boundD + boundU ) / 2.0f;
    float Xdist = ( boundR - boundL ) * scaleX / 2.0f;
    float Ydist = ( boundU - boundD ) * scaleY / 2.0f;
    // .95f to give some extra room
    // TODO make this an argument?
    float gridInnerL = centerX - (Xdist*0.95f);
    float gridInnerR = centerX + (Xdist*0.95f);
    float gridInnerU = centerY + (Ydist*0.95f);
    float gridInnerD = centerY - (Ydist*0.95f);
    float perimeterInnerL = centerX - Xdist;
    float perimeterInnerR = centerX + Xdist;
    float perimeterInnerU = centerY + Ydist;
    float perimeterInnerD = centerY - Ydist;

    //printf( "LayoutManager::focusing, inner rect %f %f %f %f\n", innerL, innerR,
    //        innerU, innerD );

    bool grid = gridArrange( gridInnerL, gridInnerR, gridInnerU, gridInnerD,
                                 true, false, true,
                                 inners );
    perimeterArrange( boundL, boundR, boundU, boundD,
                        perimeterInnerL, perimeterInnerR,
                        perimeterInnerU, perimeterInnerD,
                          outers );

    return grid;
}
