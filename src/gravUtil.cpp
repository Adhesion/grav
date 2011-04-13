/*
 * @file gravUtil.cpp
 *
 * Util functions for grav, like finding files and logging.
 *
 * Created on: Mar 16, 2011
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

#include "gravUtil.h"

#include <wx/file.h>
#include <wx/filename.h>
#include <wx/log.h>

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
        logVerbose( "gravUtil::findFile: checking %s...\n", full.c_str() );
        if ( wxFile::Exists( fullwx.c_str() ) )
        {
            logVerbose( "gravUtil::findFile: success!\n" );
            return full;
        }
    }
    logError( "gravUtil::findFile: file %s not found\n", file.c_str() );
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

void gravUtil::initLogging()
{
    // note, here we're using a logstream instead of the logstderr - output from
    // the latter wouldn't show up for some reason.
    // also note logstream may not be available in some wx builds (needs
    // wxUSE_STD_IOSTREAM / compiled with iostream support) but that's default
    // in standard builds so leaving it for now
    wxLog* logger = new wxLogStream();
    delete wxLog::SetActiveTarget( logger );
    // the delete is normal, setactivetarget returns a pointer to the old target
    // so we can get rid of it

    // set the timestamp format here
    // note this SHOULD be dependent on the locale, but AccessGrid actually
    // changes the locale itself which modifies the timestamp format.
    // Changing the locale may have other ramifications.
    // See https://grav.rc.rit.edu/wiki/KnownIssues
    wxString timestampFormat( "%H:%M:%S", wxConvUTF8 );
    wxLog::SetTimestamp( timestampFormat.c_str() );
}

void gravUtil::logVerbose( const char* str, ... )
{
    va_list args;
    va_start( args, str );
    wxLogVerbose( getWXStringFromArgs( str, args ) );
    va_end( args );
}

void gravUtil::logMessage( const char* str, ... )
{
    va_list args;
    va_start( args, str );
    wxLogMessage( getWXStringFromArgs( str, args ) );
    va_end( args );
}

void gravUtil::logStatus( const char* str, ... )
{
    va_list args;
    va_start( args, str );
    wxLogStatus( getWXStringFromArgs( str, args ) );
    va_end( args );
}

void gravUtil::logWarning( const char* str, ... )
{
    va_list args;
    va_start( args, str );
    wxLogWarning( getWXStringFromArgs( str, args ) );
    va_end( args );
}

void gravUtil::logError( const char* str, ... )
{
    va_list args;
    va_start( args, str );
    wxLogError( getWXStringFromArgs( str, args ) );
    va_end( args );
}

void gravUtil::logFatalError( const char* str, ... )
{
    va_list args;
    va_start( args, str );
    wxLogFatalError( getWXStringFromArgs( str, args ) );
    va_end( args );
}

std::string gravUtil::getVersionString()
{
    std::string out;
    out += GRAV_VERSION_MAJOR;
    out += ".";
    out += GRAV_VERSION_MINOR;
    out += ".";
    out += GRAV_VERSION_MICRO;
#ifdef GRAV_DEBUG_MODE
        out += " (Debug Build)";
#endif
    return out;
}

wxString gravUtil::getWXStringFromArgs( const char* str, va_list args )
{
    // convert incoming string+format input to final formatted form (via
    // vsprintf), and then send that result to wxLog
    // we SHOULD be able to just send the va_args to wxLog but that didn't seem
    // to parse it properly (wildly random numbers in place of %i/%u, etc.)
    wxString logString;
    char* buffer;
    buffer = new char[256];

    vsprintf( buffer, str, args );

    logString = wxString( buffer, wxConvUTF8 );
    // to remove trailing space/newline/etc. (gui log ignores it but printout
    // still has eg newlines which is annoying)
    logString = logString.Trim();
    delete[] buffer;

    return logString;
}
