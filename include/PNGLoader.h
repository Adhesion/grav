/*
 * @file PNGLoader.h
 *
 * Defines a function for loading PNG files for use as OpenGL textures.
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

#ifndef PNGLOADER_H_
#define PNGLOADER_H_

#include <string>

namespace PNGLoader
{

    /*
     * Reads in file from filename, passes it to libpng, and returns the
     * ID associated with the allocated GL texture.
     */
    GLuint loadPNG( std::string filename, int &width, int &height );

}

#endif /*PNGLOADER_H_*/
