/*
 * @file LayoutManager.h
 *
 * Contains a set of algorithms, taking in lists of objects and parameters, and
 * arranging the objects in particular ways.
 * Future versions may read from Python scripts.
 *
 * @author Andrew Ford
 * @author Ralph Bean
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

#ifndef LAYOUTMANAGER_H_
#define LAYOUTMANAGER_H_

#include <string>
#include <map>
#include <vector>

class RectangleBase;

class LayoutManager
{

public:
    LayoutManager();

    bool arrange( std::string method,
                  RectangleBase outerRect,
                  RectangleBase innerRect,
                  std::map<std::string, std::vector<RectangleBase*> > data,
                  std::map<std::string, std::string> options=std::map<std::string, std::string>());
    bool arrange( std::string method,
                  float outerL, float outerR, float outerU, float outerD,
                  float innerL, float innerR, float innerU, float innerD,
                  std::map<std::string, std::vector<RectangleBase*> > data,
                  std::map<std::string, std::string> options=std::map<std::string, std::string>());

private:
    bool focus( float outerL, float outerR, float outerU, float outerD,
                float innerL, float innerR, float innerU, float innerD,
                std::map<std::string, std::vector<RectangleBase*> > data,
                std::map<std::string, std::string> options=std::map<std::string, std::string>());

    // helper function to make an inner rect based on "aspect" and "scale"
    // options.
    // inner rect supplied is ignored.
    // like focus, must be passed data["outers"] and data["inners"]
    bool aspectFocus( float outerL, float outerR, float outerU, float outerD,
                float innerL, float innerR, float innerU, float innerD,
                std::map<std::string, std::vector<RectangleBase*> > data,
                std::map<std::string, std::string> options=std::map<std::string, std::string>());

    bool perimeterArrange( float outerL, float outerR, float outerU,
                            float outerD, float innerL, float innerR,
                            float innerU, float innerD,
                            std::map<std::string, std::vector<RectangleBase*> > data,
                            std::map<std::string, std::string> options=std::map<std::string, std::string>());

    /*
     * Arranges objects in a grid based on options.
     * Inner rectangle is ignored.
     */
    bool gridArrange( float outerL, float outerR, float outerU, float outerD,
                      float innerL, float innerR, float innerU, float innerD,
                      std::map<std::string, std::vector<RectangleBase*> > data,
                      std::map<std::string, std::string> options);
};

#endif /*LAYOUTMANAGER_H_*/
