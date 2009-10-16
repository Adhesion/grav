#ifndef PNGLOADER_H_
#define PNGLOADER_H_

/*
 * @file PNGLoader.h
 * Defines a function for loading PNG files for use as OpenGL textures.
 * @author Andrew Ford
 */

namespace PNGLoader
{
    
    /*
     * Reads in file from filename, passes it to libpng, and returns the
     * ID associated with the allocated GL texture.
     */
    GLuint loadPNG( std::string filename, int &width, int &height );
    
}

#endif /*PNGLOADER_H_*/
