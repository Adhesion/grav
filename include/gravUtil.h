/*
 * @file gravUtil.h
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

#ifndef GRAVUTIL_H_
#define GRAVUTIL_H_

// this is to prevent issues with FFmpeg, which needs __STDC_CONSTANT_MACROS to
// be defined before stdint.h is included
#include <VPMedia/VPMedia_config.h>

#include <vector>
#include <string>

#include <stdarg.h>

#include <wx/string.h>

class gravUtil
{

public:
    static gravUtil* getInstance();
    /*
     * Given a filename, search for it in the path list and return the full
     * path. If not found, returns empty string.
     */
    std::string findFile( std::string file );
    /*
     * Add a full pathname to the beginning of the path list.
     */
    void addPath( std::string path );

    /*
     * Log functions - goes straight to WX for now, abstracting just in case we
     * want to switch to something else later.
     * Static, because they don't depend on class state at all (wx log targets
     * etc. are global).
     * Currently, inputs printf format, sends final formatted string to wx log
     * functions.
     */
    static void initLogging();

    static void logVerbose( const char* str, ... );
    static void logMessage( const char* str, ... );
    static void logStatus( const char* str, ... );
    static void logWarning( const char* str, ... );
    static void logError( const char* str, ... );
    static void logFatalError( const char* str, ... );

    static std::string getVersionString();

protected:
    gravUtil();
    ~gravUtil();

private:
    static gravUtil* instance;
    /*
     * All paths in this list should have the separator at the end. addPath will
     * ensure this is the case.
     */
    std::vector<std::string> resourceDirList;
    std::string sep;

    static wxString getWXStringFromArgs( const char* str, va_list args );

};

#endif /* GRAVUTIL_H_ */
