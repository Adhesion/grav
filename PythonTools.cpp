
#include "PythonTools.h"
#include <iostream>

PythonTools::PythonTools()
{
    curModule = "";
    Py_Initialize();
}
PythonTools::~PythonTools()
{
    unload();
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
    PyObject* keyList = PyDict_Keys( d );
    PyObject* curKey;
    PyObject* curVal;
    for ( int i = 0; i < PyList_Size( keyList ); i++ )
    {
        curKey = PyList_GetItem( keyList, i );
        curVal = PyDict_GetItem( d, curKey );
        results.insert( std::pair<std::string, std::string>(
                            PyString_AsString( curKey ),
                            PyString_AsString( curVal ) ) );
    }
    return results;
}

std::vector<std::string> PythonTools::ltov( PyObject* l )
{
    PyObject* item;
    std::string str;
    std::vector<std::string> results = std::vector<std::string>();
    for ( int i = 0; i < PyList_Size(l); i++ )
    {
        item = PyList_GetItem(l, i);
        str = std::string(PyString_AsString(item));
        results.push_back(str);
    }
    return results;
}

PyObject* PythonTools::vtol( std::vector<std::string> v )
{
    PyObject *list = PyList_New(0);
    PyObject *item;
    std::vector<std::string>::iterator i;
    for ( i = v.begin(); i != v.end(); ++i )
    {
        item = PyString_FromString((*i).c_str());
        PyList_Append(list, item);
    }
    return list;
}

PyObject* PythonTools::call( std::string _script, std::string _func,
                             PyObject* args )
{
    PyObject *func, *result;
    int i = 0;
    load( _script );

    func = PyDict_GetItemString( main_d, _func.c_str() );
    if ( func == NULL ) {
        PyErr_Print();
        fprintf( stderr, "Failed to load function \"%s\"\n", _func.c_str() );
        Py_DECREF( func );
        return NULL;
    }
    if ( ! PyCallable_Check(func) ) {
        PyErr_Print();
        fprintf( stderr, "Attribute \"%s\" is not callable.\n", _func.c_str() );
        Py_DECREF( func );
        return NULL;
    }

    result = PyObject_CallObject( func, args );

    if ( result == NULL ) {
        PyErr_Print();
        fprintf( stderr, "Call failed.\n" );
        Py_DECREF( func );
        return NULL;
    }

    return result;
}

bool PythonTools::load( std::string module )
{
    // TODO add file modify time check also
    if ( module.compare( curModule ) == 0 )
    {
        return true;
    }
    else if ( curModule.compare( "" ) != 0 )
    {
        unload();
    }

    main_m = PyImport_AddModule( "__main__" );
    main_d = PyModule_GetDict( main_m );

    FILE* file_1 = fopen( module.c_str(), "r" );
    PyRun_File( file_1, module.c_str(), Py_file_input, main_d, main_d );

    curModule = module;

    return true;
}

void PythonTools::unload()
{
    Py_DECREF( main_d );
    Py_DECREF( main_m );
    curModule = "";
}

/* Just a test... */
int main(int argc, char *argv[])
{
    PythonTools ptools = PythonTools();
    /*std::vector<std::string> v = std::vector<std::string>();
    v.push_back("foobar");
    v.push_back("oh noes");
    PyObject* list = ptools.vtol(v);
    std::vector<std::string> res = ptools.ltov(list);
    for ( int i = 0; i < res.size(); i++ ) {
        fprintf(stdout, "'%s'\n", res[i].c_str());
    }
    PyObject* tuple = PyTuple_New(1);
    PyTuple_SetItem(tuple, 0, list);
    PyObject* pRes = ptools.call("py/test.py",
                                 "test_function", tuple);
    res = ptools.ltov(pRes);
    for ( int i = 0; i < res.size(); i++ ) {
        fprintf(stdout, "'%s'\n", res[i].c_str());
    }
    Py_DECREF(list);*/
    std::map<std::string, std::string> testMap;
    testMap.insert( std::pair<std::string, std::string>("foo", "bar" ) );
    testMap.insert( std::pair<std::string, std::string>("baz", "womp" ) );
    PyObject* testDict = ptools.mtod( testMap );
    std::map<std::string, std::string> testMap2 = ptools.dtom( testDict );
    std::map<std::string, std::string>::iterator it;
    for ( it = testMap2.begin(); it != testMap2.end(); ++it )
    {
        printf( "%s : %s\n", it->first.c_str(), it->second.c_str() );
    }
    PyObject* tuple = PyTuple_New( 1 );
    PyTuple_SetItem( tuple, 0, testDict );
    PyObject* pRes = ptools.call("py/test.py",
                                 "test_dict_function", tuple);
    testMap2 = ptools.dtom( pRes );
    for ( it = testMap2.begin(); it != testMap2.end(); ++it )
    {
        printf( "%s : %s\n", it->first.c_str(), it->second.c_str() );
    }
    Py_DECREF( testDict );
}
