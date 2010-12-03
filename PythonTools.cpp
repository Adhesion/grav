/**
 * @file PythonTools.h
 * Contains a bucket class with utilities to call python
 * Relied heavily on the python documentation available at:
 *      http://docs.python.org/extending/embedding.html
 *
 * To compile:
 *      g++ -g -L/usr/lib/python2.6/ -lpython2.6 -I/usr/include PythonTools.cpp
 *
 * @author Ralph Bean
 * @modified Andrew Ford
 */

#include "PythonTools.h"
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

    entryModule = "py/entry.py";
    entryFunc = "entryFunc";

    FILE* file_1 = fopen( entryModule.c_str(), "r" );
    if ( file_1 != NULL )
    {
        PyRun_File( file_1, entryModule.c_str(), Py_file_input, main_d, main_d );
        fclose( file_1 );
        init = true;
    }
    else
    {
        printf( "PythonTools::ERROR: entry script not found - python "
                "integration not available\n" );
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
                printf( "PythonTools::dtom(): WARNING: value not a string\n" );
            char* keystr = PyString_AsString( curKey );
            if ( keystr == NULL )
                printf( "PythonTools::dtom(): WARNING: key not a string\n" );
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
        for ( int i = 0; i < PyList_Size(l); i++ )
        {
            item = PyList_GetItem(l, i);
            str = std::string(PyString_AsString(item));
            results.push_back(str);
        }
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

void PythonTools::inspect_object( PyObject *o )
{
    fprintf( stdout, "Inspecting Random Object!\n");

    PyObject *item, *value, *value_s;
    std::string attr, value_stl;

    PyObject* l = PyObject_Dir( o );
    for ( int i = 0; i < PyList_Size( l ); i++ )
    {
        item = PyList_GetItem(l, i);
        attr = std::string(PyString_AsString(item));

        fprintf( stdout, "(o)%s -> ", attr.c_str() );

        value = PyObject_GetAttr( o, item );
        value_s = PyObject_Str( value );
        value_stl = std::string( PyString_AsString( value_s ) );

        fprintf( stdout, "%s\n", value_stl.c_str()  );

        Py_DECREF(value_s);
        Py_DECREF(value);
    }
    Py_DECREF(l);
}

void PythonTools::inspect_dictionary( PyObject *dict )
{
    inspect_object( main_m );

    PyObject *key, *value;
    PyObject *kstr, *vstr;
    std::string stl_kstr, stl_vstr;

    Py_ssize_t pos = 0;

    fprintf( stdout, "Inspecting Dictionary\n");
    while (PyDict_Next(dict, &pos, &key, &value)) {
        kstr = PyObject_Str(key);
        stl_kstr = PyString_AsString(kstr);

        fprintf( stdout, "(d)%s -> ", stl_kstr.c_str() );
        std::cout.flush();

        vstr = PyObject_Str(value);
        stl_vstr = PyString_AsString(vstr);

        fprintf( stdout, "%s\n", stl_vstr.c_str()  );

        Py_DECREF(kstr);
        Py_DECREF(vstr);
    }
    fprintf( stdout, "*salutes* Done Inspecting Dictionary\n");
}

PyObject* PythonTools::call( std::string _script, std::string _func,
                             PyObject* args )
{
    if ( !init )
    {
        printf( "PythonTools::call: ERROR: PyTools not initialized\n" );
        return NULL;
    }

    PyObject *func, *result;

    func = PyDict_GetItemString( main_d, entryFunc.c_str() );
    Py_INCREF( func );

    if ( func == NULL )
    {
        PyErr_Print();
        fprintf( stderr, "Failed to load function \"%s\"\n",
                    entryFunc.c_str() );
        return NULL;
    }
    if ( ! PyCallable_Check(func) )
    {
        PyErr_Print();
        fprintf( stderr, "Attribute \"%s\" is not callable.\n",
                    entryFunc.c_str() );
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
        fprintf( stderr, "Call failed.\n" );
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

/* Just a test... */
int oldmain(int argc, char *argv[])
{
    PythonTools ptools = PythonTools();
    PyObject* tuple;
    PyObject* pRes;
    std::vector<std::string> res;

    // test lists
    /*printf( "Testing python function with lists\n" );
    std::vector<std::string> v = std::vector<std::string>();
    v.push_back("foobar");
    v.push_back("oh noes");
    PyObject* list = ptools.vtol(v);
    res = ptools.ltov(list);
    printf( "List conversion to python and back:\n" );
    for ( int i = 0; i < res.size(); i++ ) {
        fprintf(stdout, "'%s'\n", res[i].c_str());
    }
    tuple = PyTuple_New( 1 );
    PyTuple_SetItem(tuple, 0, list);
    pRes = ptools.call("py/testFunctions", "test_function", tuple);
    res = ptools.ltov(pRes);
    printf( "Python function result:\n" );
    for ( int i = 0; i < res.size(); i++ ) {
        fprintf(stdout, "'%s'\n", res[i].c_str());
    }
    Py_DECREF( list );
    Py_DECREF( tuple );
    Py_DECREF( pRes );*/

    //ptools.unload();

    // test maps
    /*printf( "Testing python function with map\n" );
    std::map<std::string, std::string> testMap;
    testMap[ "foo" ] = "bar";
    testMap[ "baz" ] = "womp";
    PyObject* testDict = ptools.mtod( testMap );
    std::map<std::string, std::string> testMap2 = ptools.dtom( testDict );
    std::map<std::string, std::string>::iterator it;
    printf( "Map conversion to python and back:\n" );
    for ( it = testMap2.begin(); it != testMap2.end(); ++it )
    {
        printf( "%s : %s\n", it->first.c_str(), it->second.c_str() );
    }
    tuple = PyTuple_New( 1 );
    PyTuple_SetItem( tuple, 0, testDict );
    pRes = ptools.call("py/testFunctions", "test_dict_function", tuple);
    testMap2 = ptools.dtom( pRes );
    printf( "Python function result:\n" );
    for ( it = testMap2.begin(); it != testMap2.end(); ++it )
    {
        printf( "%s : %s\n", it->first.c_str(), it->second.c_str() );
    }
    Py_DECREF( testDict );
    Py_DECREF( tuple );
    Py_DECREF( pRes );*/

    /*tuple = PyTuple_New( 0 );
    pRes = ptools.call( "py/testFunctions", "test_unicode", tuple );
    char* charResult = PyString_AsString( pRes );
    //std::string unicodemaybe( charResult );
    //printf( "std string conv: %s\n", unicodemaybe.c_str() );
    printf( "orig: %s\n", charResult );

    Py_DECREF( tuple );
    Py_DECREF( pRes );

    tuple = PyTuple_New( 0 );
    pRes = ptools.call( "py/testFunctions", "test_unicode", tuple );
    charResult = PyString_AsString( pRes );
    //std::string unicodemaybe( charResult );
    //printf( "std string conv: %s\n", unicodemaybe.c_str() );
    printf( "orig: %s\n", charResult );

    Py_DECREF( tuple );
    Py_DECREF( pRes );*/

    /*tuple = PyTuple_New( 0 );
    pRes = ptools.call( "py/AGTools.py", "GetVenueClients", tuple );
    res = ptools.ltov( pRes );
    printf( "AGTools function result:\n" );
    for ( int i = 0; i < res.size(); i++ ) {
        fprintf(stdout, "'%s'\n", res[i].c_str());
    }

    Py_DECREF( tuple );
    Py_DECREF( pRes );*/
}

int main_2(int argc, char *argv[])
{
    PythonTools ptools = PythonTools();
    PyObject* pRes;
    std::vector<std::string> res;

    for ( int q = 0; q < 5; q++ )
    {

        printf( "Testing python function with lists\n" );
        std::vector<std::string> v = std::vector<std::string>();
        v.push_back("foobar");
        v.push_back("oh noes");
        PyObject* list = ptools.vtol(v);
        res = ptools.ltov(list);
        printf( "List conversion to python and back:\n" );
        for ( int i = 0; i < res.size(); i++ ) {
            fprintf(stdout, "'%s'\n", res[i].c_str());
        }
        pRes = ptools.call("testFunctions", "test_function", list);
        res = ptools.ltov(pRes);
        printf( "Python function result:\n" );
        for ( int i = 0; i < res.size(); i++ ) {
            fprintf(stdout, "'%s'\n", res[i].c_str());
        }
        Py_DECREF( pRes );

        printf( "Testing python function with lists\n" );
        v = std::vector<std::string>();
        v.push_back("foobar");
        v.push_back("oh noes");
        list = ptools.vtol(v);
        res = ptools.ltov(list);
        printf( "List conversion to python and back:\n" );
        for ( int i = 0; i < res.size(); i++ ) {
            fprintf(stdout, "'%s'\n", res[i].c_str());
        }
        pRes = ptools.call("/home/user/src/grav/py/testFunctions.py", "test_function", list);
        res = ptools.ltov(pRes);
        printf( "Python function result:\n" );
        for ( int i = 0; i < res.size(); i++ ) {
            fprintf(stdout, "'%s'\n", res[i].c_str());
        }
        Py_DECREF( pRes );

        printf( "Testing AGTools\n" );
        //tuple = PyTuple_New( 0 );
        pRes = ptools.call( "AGTools", "GetVenueClients", NULL );
        res = ptools.ltov( pRes );
        printf( "AGTools getvenueclient function result:\n" );
        for ( int i = 0; i < res.size(); i++ ) {
            fprintf(stdout, "'%s'\n", res[i].c_str());
        }

        //Py_DECREF( tuple );
        Py_DECREF( pRes );

        sleep(1);
    }
}

int loopmain(int argc, char** argv)
{
    PythonTools ptools = PythonTools();
    PyObject* pRes;

    pRes = ptools.call( "AGTools", "GetVenueClients", NULL );
    std::vector<std::string> clientURLs = ptools.ltov( pRes );
    Py_DECREF( pRes );
    std::string clientURL = clientURLs[0];

    pRes = ptools.call( "AGTools", "GetExits", clientURL );
    std::map<std::string, std::string> exitMap = ptools.dtom( pRes );
    Py_DECREF( pRes );

    for ( int i = 0; i < 1000000; i++ )
    {
        printf( "\n\tOn iteration %i\n", i );
        int r = rand() % exitMap.size();
        std::map<std::string, std::string>::iterator it = exitMap.begin();
        while ( r > 0 )
        {
            it++;
            r--;
        }

        if ( it->first.compare( "Educational Institution Lobby" ) == 0 )
        {
            it++;
            if ( it == exitMap.end() )
                it = exitMap.begin();
        }

        // enter venue
        PyObject* args = PyTuple_New( 2 );
        PyTuple_SetItem( args, 0, PyString_FromString( clientURL.c_str() ) );
        PyTuple_SetItem( args, 1, PyString_FromString( it->second.c_str() ) );

        ptools.call( "AGTools", "EnterVenue", args );

        // update exits
        pRes = ptools.call( "AGTools", "GetExits", clientURL );
        exitMap = ptools.dtom( pRes );
        Py_DECREF( pRes );

        std::string type = "video";
        args = PyTuple_New( 2 );
        PyTuple_SetItem( args, 0, PyString_FromString( clientURL.c_str() ) );
        PyTuple_SetItem( args, 1, PyString_FromString( type.c_str() ) );

        PyObject* res = ptools.call( "AGTools", "GetFormattedVenueStreams", args );
        std::map<std::string, std::string> currentVenueStreams = ptools.dtom( res );
        Py_DECREF( res );

        sleep( 2 );
    }
}
