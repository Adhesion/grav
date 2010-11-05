#ifndef __PYTHONTOOLS_H_
#define __PYTHONTOOLS_H_

/**
 * @file PythonTools.h
 * Contains a bucket class with utilities to call python
 * Relied heavily on the python documentation available at:
 *      http://docs.python.org/extending/embedding.html
 *
 * To compile:
 *      g++ -L/usr/lib/python2.6/ -lpython2.6 -I/usr/include PythonTools.cpp
 *
 * @author Ralph Bean
 *
 */

#include <python2.6/Python.h>
#include <string>
#include <vector>
#include <map>

class PythonTools
{

public:
    PythonTools();
    ~PythonTools();

    PyObject* call( std::string module, std::string func, PyObject* args );

    /* Map to Dict */
    PyObject* mtod( std::map<std::string, std::string> m );
    std::map<std::string, std::string> dtom( PyObject* d );

    /* Vector to List */
    PyObject* vtol( std::vector<std::string> v);
    std::vector<std::string> ltov( PyObject* l );
};

#endif /*PYTHONTOOLS_H_*/
