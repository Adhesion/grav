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


bool LayoutManager::arrange( std::string method,
                             RectangleBase outerRect,
                             RectangleBase innerRect,
                             std::vector<RectangleBase*> objects,
                             std::map<std::string, std::string> options)
{
    float outerL = outerRect.getLBound();
    float outerR = outerRect.getRBound();
    float outerU = outerRect.getUBound();
    float outerD = outerRect.getDBound();

    float innerL = innerRect.getLBound();
    float innerR = innerRect.getRBound();
    float innerU = innerRect.getUBound();
    float innerD = innerRect.getDBound();

    return arrange(method,
            outerL, outerR, outerU, outerD,
            innerL, innerR, innerU, innerD,
            objects, options);
}

bool LayoutManager::arrange( std::string method,
                             float outerL, float outerR,
                             float outerU, float outerD,
                             float innerL, float innerR,
                             float innerU, float innerD,
                             std::vector<RectangleBase*> objects,
                             std::map<std::string, std::string> options)
{
    typedef bool (LayoutManager::*fn_ptr)(
        float oL, float oR, float oU, float oD,
        float iL, float iR, float iU, float iD,
        std::vector<RectangleBase*> objs,
        std::map<std::string, std::string> opts);

    std::map<std::string, fn_ptr> lookup;
    lookup["perimeter"] = &LayoutManager::perimeterArrange;
    lookup["grid"]      = &LayoutManager::gridArrange;

    if ( lookup.find(method) == lookup.end() )
    {
        printf( "ZOMG:::: a huge error should be thrown here!!!\n" );
        return false; // double false !!!
    }
    return (this->*lookup[method])( outerL, outerR, outerU, outerD,
                                    innerL, innerR, innerU, innerD,
                                    objects, options );
}


bool LayoutManager::perimeterArrange( RectangleBase outerRect,
                                        RectangleBase innerRect,
                                        std::vector<RectangleBase*> objects,
                                        std::map<std::string, std::string> opts)
{
    float outerL = outerRect.getLBound();
    float outerR = outerRect.getRBound();
    float outerU = outerRect.getUBound();
    float outerD = outerRect.getDBound();
    
    float innerL = innerRect.getLBound();
    float innerR = innerRect.getRBound();
    float innerU = innerRect.getUBound();
    float innerD = innerRect.getDBound();
    
    return perimeterArrange( outerL, outerR, outerU, outerD, innerL, innerR,
                                innerU, innerD, objects );
}

bool LayoutManager::perimeterArrange( float outerL, float outerR,
                                        float outerU, float outerD,
                                        float innerL, float innerR,
                                        float innerU, float innerD,
                                        std::vector<RectangleBase*> objects,
                                        std::map<std::string, std::string> opts)
{
    //printf( "LayoutManager::perimeter: outer inners: %f,%f %f,%f\n",
    //        outerL, outerR, outerU, outerD );

    float topRatio = (innerR-innerL) / ((outerU-outerD)+(innerR-innerL));
    float sideRatio = (outerU-outerD) / ((outerU-outerD)+(innerR-innerL));
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
        gridArrange( innerL, innerR, outerU-0.8f, innerU, true, false, true,
                     topObjs, topNum, 1 );
    }

    end = topNum + sideNum;

    if ( sideNum > 0 )
    {
        //printf( "arranging objects %d to %d to right\n", topNum, end-1 );
        for ( int i = topNum; i < end; i++ )
            rightObjs.push_back( objects[i] );
        gridArrange( innerR, outerR, outerU, outerD, false, true, true,
                     rightObjs, 1, sideNum );
    }

    end = topNum + sideNum + bottomNum;

    if ( bottomNum > 0 )
    {
        //printf( "arranging objects %d to %d to bottom\n", topNum + sideNum,
        //        end-1 );
        for ( int i = end-1; i >= topNum + sideNum; i-- )
            bottomObjs.push_back( objects[i] );
        gridArrange( innerL, innerR, innerD, outerD, true, false, true,
                     bottomObjs, bottomNum, 1 );
    }

    if ( sideNum > 0 )
    {
        //printf( "arranging objects %d to %d to left\n",
        //        topNum + sideNum + bottomNum, objects.size()-1 );
        for ( int i = objects.size()-1; i >= topNum + sideNum + bottomNum; i-- )
            leftObjs.push_back( objects[i] );
        gridArrange( outerL, innerL, outerU, outerD, false, true, true,
                     leftObjs, 1, sideNum );
    }
    return true;
}

bool LayoutManager::gridArrange( RectangleBase innerRect,
                                    bool horiz, bool edge, bool resize,
                                    std::vector<RectangleBase*> objects,
                                    int numX, int numY )
{
    float innerL = innerRect.getLBound();
    float innerR = innerRect.getRBound();
    float innerU = innerRect.getUBound();
    float innerD = innerRect.getDBound();
    
    return gridArrange( innerL, innerR, innerU, innerD, horiz, edge,
                            resize, objects, numX, numY );
}

// Little utility... should be phased out with a better usage of std::map
bool str2bool(std::string str) { return str == "True" ? True : False; }
int str2int(std::string str) { return atoi(str.c_str()); }

bool LayoutManager::gridArrange(float outerL, float outerR,
                                float outerU, float outerD,
                                float innerL, float innerR,
                                float innerU, float innerD,
                                std::vector<RectangleBase*> objects,
                                std::map<std::string, std::string> opts )
{
    // Setup opts defaults
    std::map<std::string, std::string> dflt = \
        std::map<std::string, std::string>();
    dflt["horiz"] = "True"; dflt["edge"] = "False"; dflt["resize"] = "True";
    dflt["numX"] = "0";     dflt["numY"] = "0";

    // Apply the opts defaults to opts
    for (std::map<std::string,std::string>::iterator i = dflt.begin();
         i != dflt.end(); i++)
    {
        // If the value wasn't specified by our caller, then load the default
        if ( opts.find(i->first) == opts.end() ) 
            opts[i->first] = i->second;
    }

    return gridArrange(innerL, innerR, innerU, innerD,
                       str2bool(opts["horiz"]),
                       str2bool(opts["edge"]),
                       str2bool(opts["resize"]),
                       objects,
                       str2int(opts["numX"]),
                       str2int(opts["numY"]));
}
bool LayoutManager::gridArrange( float innerL, float innerR, float innerU,
                                    float innerD,
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
        fullscreen( innerL, innerR, innerU, innerD, objects[0] );
        return true;
    }
    
    // printf( "grid:inners: %f,%f %f,%f\n", innerL, innerR, innerU, innerD );
    
    float span; // height of rows if going horizontally,
                // width of columns if going vertically
    float stride; // distance to move each time
    float curX, curY;
    float edgeL = innerL, edgeR = innerR, edgeU = innerU, edgeD = innerD;
    
    // set up span and stride, etc differently for horizontal vs vertical
    // arrangement
    if ( horiz )
    {
        span = (innerU-innerD) / numY;
        stride = (innerR-innerL) / numX;
        
        edgeL = innerL + 0.2f + (stride / 2);
        edgeR = innerR - 0.2f - (stride / 2);
        //printf( "grid: edges are %f,%f\n", edgeL, edgeR );
        if ( edge ) stride = (edgeR-edgeL) / std::max(1, (numX-1));
        
        curY = innerU - (span/2.0f);
        
        if ( numX == 1 )
            curX = (innerR+innerL)/2.0f;
        else
        {
            if ( edge )
                curX = edgeL;
            else
                curX = innerL + (stride/2.0f);
        }
    }
    else
    {
        span = (innerR-innerL) / numX;
        stride = (innerU-innerD) / numY;
        
        edgeU = innerU - 0.2f - (stride / 2);
        edgeD = innerD + 0.2f + (stride / 2);
        if ( edge ) stride = (edgeU-edgeD) / std::max(1, (numY-1));

        curX = innerL + (span/2.0f);
        
        if ( numY == 1 )
            curY = (innerU+innerD)/2.0f;
        else
        {
            if ( edge )
                curY = edgeU;
            else
                curY = innerU - (stride/2.0f);
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
                        stride = (innerR-innerL) / (objectsLeft);
                }
                curX = innerL + (stride/2.0f);
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
                        stride = innerU-innerD / (objectsLeft+1);
                }
                curY = innerU - (stride/2.0f);
            }
        }
    }
    
    return true;
}

bool LayoutManager::fullscreen( RectangleBase innerRect, RectangleBase* object )
{
    float innerL = innerRect.getLBound();
    float innerR = innerRect.getRBound();
    float innerU = innerRect.getUBound();
    float innerD = innerRect.getDBound();
    
    return fullscreen( innerL, innerR, innerU, innerD, object );
}

bool LayoutManager::fullscreen( float innerL, float innerR, float innerU,
                                    float innerD, RectangleBase* object )
{
    float spaceAspect = fabs((innerR-innerL)/(innerU-innerD));
    float objectAspect = object->getTotalWidth()/object->getTotalHeight();
    //printf( "LayoutManager::fullscreen: aspects are %f in %f\n", objectAspect, spaceAspect );

    if ( ( spaceAspect - objectAspect ) > 0.01f )
    {
        //printf( "LayoutManager::fullscreen: setting height to %f\n",
        //        innerU-innerD );
        object->setTotalHeight( innerU-innerD );
    }
    else
    {
        //printf( "LayoutManager::fullscreen: setting width to %f\n",
        //        innerR-innerL );
        object->setTotalWidth( innerR-innerL );
    }

    // TODO need to change this if getCenterOffsetX() is ever meaningful
    object->move( (innerR+innerL)/2.0f, ((innerU+innerD)/2.0f) -
                    object->getCenterOffsetY() );

    return true;
}

bool LayoutManager::focus( RectangleBase innerRect,
                std::vector<RectangleBase*> outers,
                std::vector<RectangleBase*> inners,
                float scaleX, float scaleY )
{
    float innerL = innerRect.getLBound();
    float innerR = innerRect.getRBound();
    float innerU = innerRect.getUBound();
    float innerD = innerRect.getDBound();

    //printf( "LayoutManager::focusing, outer rect %f %f %f %f\n", innerL, innerR,
    //        innerU, innerD );

    return focus( innerL, innerR, innerU, innerD, outers, inners,
                        scaleX, scaleY );
}

bool LayoutManager::focus( float innerL, float innerR, float innerU,
                float innerD, std::vector<RectangleBase*> outers,
                std::vector<RectangleBase*> inners,
                float scaleX, float scaleY )
{
    float centerX = ( innerL + innerR ) / 2.0f;
    float centerY = ( innerD + innerU ) / 2.0f;
    float Xdist = ( innerR - innerL ) * scaleX / 2.0f;
    float Ydist = ( innerU - innerD ) * scaleY / 2.0f;
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
    perimeterArrange( innerL, innerR, innerU, innerD,
                        perimeterInnerL, perimeterInnerR,
                        perimeterInnerU, perimeterInnerD,
                          outers );

    return grid;
}
