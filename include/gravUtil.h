/*
 * gravUtil.h
 *
 * Simple util functions for grav, like finding files.
 *
 * Created on: Mar 16, 2011
 * @author Andrew Ford
 */

#ifndef GRAVUTIL_H_
#define GRAVUTIL_H_

#include <vector>
#include <string>

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

protected:
    gravUtil();
    ~gravUtil();

private:
    static gravUtil* instance;
    std::vector<std::string> resourceDirList;

};

#endif /* GRAVUTIL_H_ */
