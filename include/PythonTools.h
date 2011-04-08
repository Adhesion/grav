/*
 * @file PythonTools.h
 *
 * Contains a bucket class with utilities to call python
 * Relied heavily on the python documentation available at:
 *      http://docs.python.org/extending/embedding.html
 *
 * @author Ralph Bean
 * @modified Andrew Ford
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

#ifndef __PYTHONTOOLS_H_
#define __PYTHONTOOLS_H_

#include <Python.h>
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
    PyObject* vtol( std::vector<std::string> v );
    std::vector<std::string> ltov( PyObject* l );

    /* Print contents to stdout */
    void inspect_dictionary(PyObject *dict);
    void inspect_object(PyObject *obj);

private:
    PyObject *main_m, *main_d; // dictionary/globals/locals for python
    std::string entryModule;
    std::string entryFunc;

    bool init;

};

#endif /*PYTHONTOOLS_H_*/
