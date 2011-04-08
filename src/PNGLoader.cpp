/*
 * @file PNGLoader.cpp
 *
 * Defines a function for loading PNG files for use as OpenGL textures.
 * Implementation based on one from
 * http://en.wikibooks.org/wiki/OpenGL_Programming/Intermediate/Textures
 * which is licensed under a Creative Commons by-sa 3.0 license.
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

#include <string>
#include <cstdio>
#include <png.h>

#include "GLUtil.h"
#include "PNGLoader.h"
#include "gravUtil.h"

GLuint PNGLoader::loadPNG( std::string filename, int &width, int &height )
{
    int size = 0;
    glGetIntegerv( GL_MAX_TEXTURE_SIZE, &size );
    gravUtil::logVerbose( "PNGLoader::loadPNG: max tex size is %i", size );

    png_byte PNGheader[8];

    // open the texture as a binary file
    FILE* texfile = fopen( filename.c_str(), "rb" );
    if ( !texfile )
    {
        gravUtil::logError( "PNGLoader::loadPNG: error opening file %s\n",
                filename.c_str() );
        return 0;
    }

    size_t retval = fread( PNGheader, 1, 8, texfile );
    if ( retval == 0 )
    {
        gravUtil::logError( "PNGLoader::loadPNG: error reading file %s?\n",
                filename.c_str() );
        return 0;
    }

    // check the header
    int pngTest = !png_sig_cmp( PNGheader, 0, 8 );
    if ( !pngTest )
    {
        fclose( texfile );
        return 0;
    }

    // make the main png struct
    png_structp png = png_create_read_struct( PNG_LIBPNG_VER_STRING, NULL,
        NULL, NULL );
    if ( !png )
    {
        fclose( texfile );
        return 0;
    }

    // make the info struct
    png_infop pngInfo = png_create_info_struct( png );
    if ( !pngInfo )
    {
        png_destroy_read_struct( &png, (png_infopp)NULL, (png_infopp)NULL );
        fclose( texfile );
        return 0;
    }

    // make the end struct
    png_infop pngEnd = png_create_info_struct( png );
    if ( !pngEnd )
    {
        png_destroy_read_struct( &png, &pngInfo, (png_infopp)NULL );
        fclose( texfile );
        return 0;
    }

    // weird png error stuff...
    if ( setjmp( png_jmpbuf( png ) ) )
    {
        png_destroy_read_struct( &png, &pngInfo, &pngEnd );
        fclose ( texfile );
        return 0;
    }

    // initialize reading, set that we already read the header & read the
    // rest of the info block
    png_init_io( png, texfile );
    png_set_sig_bytes( png, 8 );
    png_read_info( png, pngInfo );

    // read in bit depth/color/width/height info
    int bitDepth, colorType;
    png_uint_32 iwidth, iheight;

    png_get_IHDR( png, pngInfo, &iwidth, &iheight, &bitDepth, &colorType,
                    NULL, NULL, NULL );
    width = iwidth; height = iheight;

    gravUtil::logVerbose( "PNGLoader::loadPNG: bitDepth: %i, colorType: %i, "
            "RGBA: %i\n", bitDepth, colorType, PNG_COLOR_TYPE_RGBA );

    int pwidth = GLUtil::getInstance()->pow2( iwidth );
    int pheight = GLUtil::getInstance()->pow2( iheight );
    gravUtil::logVerbose( "PNGLoader::loadPNG: read in PNG: image dimensions: "
            "%ux%u, pow2 dimensions: %ux%u\n",
            (unsigned int)iwidth, (unsigned int)iheight,
            (unsigned int)pwidth, (unsigned int)pheight );

    // update the info struct
    png_read_update_info( png, pngInfo );
    int rowBytes = png_get_rowbytes( png, pngInfo );

    png_byte* image = new png_byte[ rowBytes * iheight ];

    // set the pointers for libpng to read the image row-by-row
    png_bytep* rowPointers = new png_bytep[ iheight ];
    for ( unsigned int i = 0; i < iheight; i++ )
        rowPointers[iheight - 1 - i] = image + (i * rowBytes);

    // read in the image
    png_read_image( png, rowPointers );

    GLenum  gl_error = glGetError();
    for ( ; (gl_error); gl_error = glGetError() )
    {
        gravUtil::logError( "PNGLoader::loadPNG: GLError: %s\n",
                (const GLchar*)gluErrorString( gl_error ) );
    }

    GLuint texID;
    glGenTextures( 1, &texID );
    glBindTexture( GL_TEXTURE_2D, texID );

    // allocate a buffer for the pow2 size
    unsigned char *buffer = new unsigned char[pwidth * pheight * 4];
    gravUtil::logVerbose( "PNGLoader::loadPNG: made buffer, "
            "allocating texture\n" );
    memset( buffer, 128, pwidth * pheight * 4 );

    gl_error = glGetError();
    for ( ; (gl_error); gl_error = glGetError() )
    {
        gravUtil::logError( "PNGLoader::loadPNG: GLError: %s\n",
                (const GLchar*)gluErrorString( gl_error ) );
    }

    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glPixelStorei( GL_UNPACK_ROW_LENGTH, pwidth );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, pwidth, pheight, 0, GL_RGBA,
                    GL_UNSIGNED_BYTE, (GLvoid*)buffer );

    glPixelStorei( GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei( GL_UNPACK_ROW_LENGTH, iwidth);

    gravUtil::logVerbose( "PNGLoader::loadPNG: putting PNG in texture area\n" );

    // put the actual image in a sub-area of the pow2 memory area
    glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, iwidth, iheight, GL_RGBA,
                    GL_UNSIGNED_BYTE, (GLvoid*)image );

    gl_error = glGetError();
    for ( ; (gl_error); gl_error = glGetError() )
    {
        gravUtil::logError("PNGLoader::loadPNG: GLError: %s\n",
                (const GLchar*)gluErrorString( gl_error ) );
    }

    gravUtil::logVerbose( "PNGLoader::loadPNG: PNGLoader::generated ID is %i\n",
            texID );

    png_destroy_read_struct( &png, &pngInfo, &pngEnd );
    delete[] image;
    delete[] rowPointers;
    delete[] buffer;
    fclose( texfile );

    return texID;
}
