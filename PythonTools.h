#ifndef __PYTHONTOOLS_H_
#define __PYTHONTOOLS_H_

/**
 * @file PythonTools.h
 * Contains a bucket class with utilities to call python
 * Relied heavily on the python documentation available at:
 *      http://docs.python.org/extending/embedding.html
 *
 * @author Ralph Bean
 *
 */

#include <python2.6/Python.h>
#include <string>
#include <vector>

class PythonTools
{
    
public:
    PythonTools();
    ~PythonTools();

    PyObject* call( std::string module, std::string func, PyObject* args );

    /* Map to Dict */
    //PyDictObject* mtod(std::map<> m);

    /* Vector to List */
    PyObject* vtol(std::vector<std::string> v);
    std::vector<std::string> ltov(PyObject*);
};

#endif /*PYTHONTOOLS_H_*/
