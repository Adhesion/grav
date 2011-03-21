/*
 * gravUtil.cpp
 *
 * Simple util functions for grav, like finding files.
 *
 * Created on: Mar 16, 2011
 * @author Andrew Ford
 */

#include "gravUtil.h"

#include <wx/file.h>
#include <wx/filename.h>

gravUtil* gravUtil::instance = NULL;

gravUtil* gravUtil::getInstance()
{
    if ( instance == NULL )
    {
        instance = new gravUtil();
    }
    return instance;
}

gravUtil::gravUtil()
{
    sep = wxFileName::GetPathSeparator();
    std::string subdir = "py";
    resourceDirList.push_back( "." + sep );
    resourceDirList.push_back( "." + sep + subdir + sep );
    resourceDirList.push_back( GRAV_RESOURCE_DIR );
    resourceDirList.push_back( GRAV_RESOURCE_DIR + subdir + sep );
}

gravUtil::~gravUtil()
{

}

std::string gravUtil::findFile( std::string file )
{
    for ( std::vector<std::string>::iterator i = resourceDirList.begin();
            i != resourceDirList.end(); ++i )
    {
        std::string full = (*i) + file;
        wxString fullwx = wxString( full.c_str(), wxConvUTF8 );
        printf( "gravUtil::findFile: checking %s...\n", full.c_str() );
        if ( wxFile::Exists( fullwx.c_str() ) )
        {
            printf( "gravUtil::findFile: success!\n" );
            return full;
        }
    }
    printf( "gravUtil::findFile: file %s not found\n", file.c_str() );
    return "";
}

void gravUtil::addPath( std::string path )
{
    std::vector<std::string>::iterator start = resourceDirList.begin();
    // check if path separator is at end of input, if not, add it
    if ( *(path.rbegin()) != *(sep.begin()) )
    {
        path += sep;
    }
    resourceDirList.insert( start, path );
}
