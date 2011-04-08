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

#include "PythonTools.h"
#include "gravUtil.h"
#include <iostream>
#include <cstdlib>

PythonTools::PythonTools()
{
    init = false;
    main_m = NULL;
    main_d = NULL;
    Py_Initialize();
    main_m = PyImport_AddModule( "__main__" );
    main_d = PyModule_GetDict( main_m );

    gravUtil* util = gravUtil::getInstance();
    entryModule = util->findFile( "gravEntry.py" );
    entryFunc = "entryFunc";

    bool found = false;
    if ( entryModule.compare( "" ) != 0 )
    {
        FILE* file_1 = fopen( entryModule.c_str(), "r" );
        found = file_1 != NULL;
        if ( found )
        {
            PyRun_File( file_1, entryModule.c_str(), Py_file_input, main_d,
                            main_d );
            fclose( file_1 );
            init = true;
        }
    }

    if ( !found )
    {
        gravUtil::logWarning( "PythonTools::init: entry script not found - "
                "python integration not available\n" );
    }
}

PythonTools::~PythonTools()
{
    Py_Finalize();
}

PyObject* PythonTools::mtod( std::map<std::string, std::string> m )
{
    PyObject* dict = PyDict_New();
    PyObject* curKey;
    PyObject* curVal;
    std::map<std::string, std::string>::iterator it;
    for ( it = m.begin(); it != m.end(); ++it )
    {
        curKey = PyString_FromString( it->first.c_str() );
        curVal = PyString_FromString( it->second.c_str() );
        PyDict_SetItem( dict, curKey, curVal );
    }
    return dict;
}

std::map<std::string, std::string> PythonTools::dtom( PyObject* d )
{
    std::map<std::string, std::string> results;
    if ( d != NULL && PyDict_Check( d ) )
    {
        PyObject* keyList = PyDict_Keys( d );
        PyObject* curKey;
        PyObject* curVal;
        for ( int i = 0; i < PyList_Size( keyList ); i++ )
        {
            curKey = PyList_GetItem( keyList, i );
            curVal = PyDict_GetItem( d, curKey );
            char* valstr = PyString_AsString( curVal );
            if ( valstr == NULL )
                gravUtil::logWarning( "PythonTools::dtom(): "
                        "value not a string\n" );
            char* keystr = PyString_AsString( curKey );
            if ( keystr == NULL )
                gravUtil::logWarning( "PythonTools::dtom(): "
                        "key not a string\n" );
            if ( keystr != NULL && valstr != NULL )
                results[ keystr ] = valstr;
        }
    }
    return results;
}

std::vector<std::string> PythonTools::ltov( PyObject* l )
{
    std::vector<std::string> results = std::vector<std::string>();
    if ( l != NULL )
    {
        PyObject* item;
        std::string str;
        for ( int i = 0; i < PyList_Size( l ); i++ )
        {
            item = PyList_GetItem( l, i );
            str = std::string( PyString_AsString( item ) );
            results.push_back( str );
        }
    }
    return results;
}

PyObject* PythonTools::vtol( std::vector<std::string> v )
{
    PyObject *list = PyList_New( 0 );
    PyObject *item;
    std::vector<std::string>::iterator i;
    for ( i = v.begin(); i != v.end(); ++i )
    {
        item = PyString_FromString( (*i).c_str() );
        PyList_Append( list, item );
    }
    return list;
}

void PythonTools::inspect_object( PyObject *o )
{
    gravUtil::logMessage( "PythonTools::Inspecting Random Object!\n" );

    PyObject *item, *value, *value_s;
    std::string attr, value_stl;

    PyObject* l = PyObject_Dir( o );
    for ( int i = 0; i < PyList_Size( l ); i++ )
    {
        item = PyList_GetItem( l, i );
        attr = std::string( PyString_AsString( item ) );

        value = PyObject_GetAttr( o, item );
        value_s = PyObject_Str( value );
        value_stl = std::string( PyString_AsString( value_s ) );

        gravUtil::logMessage( "\t(o)%s -> %s\n", attr.c_str(),
                value_stl.c_str() );

        Py_DECREF( value_s );
        Py_DECREF( value );
    }
    Py_DECREF( l );

    gravUtil::logMessage( "PythonTools::Done Inspecting Random Object\n" );
}

void PythonTools::inspect_dictionary( PyObject *dict )
{
    inspect_object( main_m );

    PyObject *key, *value;
    PyObject *kstr, *vstr;
    std::string stl_kstr, stl_vstr;

    Py_ssize_t pos = 0;

    gravUtil::logMessage( "PythonTools::Inspecting Dictionary\n" );
    while ( PyDict_Next( dict, &pos, &key, &value ) )
    {
        kstr = PyObject_Str( key );
        stl_kstr = PyString_AsString( kstr );

        vstr = PyObject_Str( value );
        stl_vstr = PyString_AsString( vstr );

        gravUtil::logMessage( "\t(d)%s -> %s\n", stl_kstr.c_str(),
                stl_vstr.c_str() );

        Py_DECREF( kstr );
        Py_DECREF( vstr );
    }

    gravUtil::logMessage( "PythonTools::Done Inspecting Dictionary\n" );
}

PyObject* PythonTools::call( std::string _script, std::string _func,
                             PyObject* args )
{
    if ( !init )
    {
        gravUtil::logError( "PythonTools::call: PyTools not initialized\n" );
        return NULL;
    }

    PyObject *func, *result;

    func = PyDict_GetItemString( main_d, entryFunc.c_str() );

    if ( func == NULL )
    {
        PyErr_Print();
        gravUtil::logError( "PythonTools::call: Failed to load function "
                "\"%s\"\n", entryFunc.c_str() );
        return NULL;
    }
    Py_INCREF( func );
    if ( ! PyCallable_Check(func) )
    {
        PyErr_Print();
        gravUtil::logError( "PythonTools::call: Attribute \"%s\" is not "
                "callable\n", entryFunc.c_str() );
        Py_DECREF( func );
        return NULL;
    }

    PyObject* entryArgs;
    if ( args == NULL )
    {
        entryArgs = PyTuple_New( 2 );
        PyTuple_SetItem( entryArgs, 0, PyString_FromString( _script.c_str() ) );
        PyTuple_SetItem( entryArgs, 1, PyString_FromString( _func.c_str() ) );
    }
    else if ( PyTuple_Check( args ) )
    {
        int num = 2 + PyTuple_Size( args );
        entryArgs = PyTuple_New( num );
        PyTuple_SetItem( entryArgs, 0, PyString_FromString( _script.c_str() ) );
        PyTuple_SetItem( entryArgs, 1, PyString_FromString( _func.c_str() ) );
        for( int i = 0; i < PyTuple_Size( args ); i++ )
        {
            PyObject* temp = PyTuple_GetItem( args, i );
            Py_INCREF( temp );
            // because setitem will steal temp, and getitem returns borrowed, we
            // need to incref...?
            PyTuple_SetItem( entryArgs, i+2, temp );
        }
    }
    else
    {
        entryArgs = PyTuple_New( 3 );
        PyTuple_SetItem( entryArgs, 0, PyString_FromString( _script.c_str() ) );
        PyTuple_SetItem( entryArgs, 1, PyString_FromString( _func.c_str() ) );
        PyTuple_SetItem( entryArgs, 2, args );
    }

    result = PyObject_CallObject( func, entryArgs );

    if ( result == NULL )
    {
        PyErr_Print();
        gravUtil::logError( "PythonTools::call: Call failed\n" );
        Py_DECREF( func );
        Py_DECREF( entryArgs );
        return NULL;
    }

    Py_DECREF( func );
    Py_DECREF( entryArgs );
    return result;
}

PyObject* PythonTools::call( std::string _script, std::string _func,
                                std::string arg )
{
    PyObject* str = PyString_FromString( arg.c_str() );
    PyObject* pRes;

    pRes = call( _script, _func, str );
    return pRes;
}

PyObject* PythonTools::call( std::string _script, std::string _func )
{
    PyObject* pRes;
    pRes = call( _script, _func, NULL );
    return pRes;
}
