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
 * @modified Andrew Ford
 */

#ifndef __PYTHONTOOLS_H_
#define __PYTHONTOOLS_H_

#include <python2.6/Python.h>
#include <string>
#include <vector>
#include <map>

class PythonTools
{

public:
    PythonTools();
    ~PythonTools();

    PyObject* call( std::string _script, std::string _func, PyObject* args );
    PyObject* call( std::string _script, std::string _func, std::string arg );
    PyObject* call( std::string _script, std::string _func );

    /* Map to Dict */
    PyObject* mtod( std::map<std::string, std::string> m );
    std::map<std::string, std::string> dtom( PyObject* d );

    /* Vector to List */
    PyObject* vtol( std::vector<std::string> v);
    std::vector<std::string> ltov( PyObject* l );

    /* Print contents to stdout */
    void inspect_dictionary(PyObject *dict);
    void inspect_object(PyObject *obj);

private:
    PyObject *main_m, *main_d; // dictionary/globals/locals for python
    std::string entryModule;
    std::string entryFunc;

};

#endif /*PYTHONTOOLS_H_*/
