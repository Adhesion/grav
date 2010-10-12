
#include "PythonTools.h"
#include <iostream>

PythonTools::PythonTools()
{
    Py_Initialize();
}
PythonTools::~PythonTools()
{
    Py_Finalize();
}

std::vector<std::string> PythonTools::ltov(PyObject* l)
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

PyObject* PythonTools::vtol(std::vector<std::string> v)
{
    PyObject *list = PyList_New(0);
    PyObject *item;
    std::vector<std::string>::iterator i;
    for ( i = v.begin(); i != v.end(); i++ )
    {
        item = PyString_FromString((*i).c_str());
        PyList_Append(list, item);
    }
    return list;
}

PyObject* PythonTools::call( std::string _script, std::string _func,
                             PyObject* args )
{
    PyObject *main_m, *main_d;
    PyObject *func, *result;
    int i = 0;
    main_m = PyImport_AddModule("__main__");
    main_d = PyModule_GetDict(main_m);
    
    FILE* file_1 = fopen(_script.c_str(), "r");
    PyRun_File(file_1, _script.c_str(), Py_file_input, main_d, main_d);

    func = PyDict_GetItemString(main_d, _func.c_str());
    if ( func == NULL ) {
        PyErr_Print();
        fprintf(stderr, "Failed to load function \"%s\"\n", _func.c_str());
        Py_DECREF(main_d);  Py_DECREF(main_m);  Py_DECREF(func);
        return NULL;
    }
    if ( ! PyCallable_Check(func) ) {
        PyErr_Print();
        fprintf(stderr, "Attribute \"%s\" is not callable.\n", _func.c_str());
        Py_DECREF(main_d);  Py_DECREF(main_m);  Py_DECREF(func);
        return NULL;
    }

    result = PyObject_CallObject(func, args);
    
    if ( result == NULL ) {
        PyErr_Print();
        fprintf(stderr, "Call failed.\n");
        Py_DECREF(main_d);  Py_DECREF(main_m);  Py_DECREF(func);
        return NULL;
    }

    return result;
}

/* Just a test... */
int main(int argc, char *argv[])
{
    PythonTools ptools = PythonTools();
    std::vector<std::string> v = std::vector<std::string>();
    v.push_back("foobar");
    v.push_back("oh noes");
    PyObject* list = ptools.vtol(v);
    std::vector<std::string> res = ptools.ltov(list);
    for ( int i = 0; i < res.size(); i++ ) {
        fprintf(stdout, "'%s'\n", res[i].c_str());
    }
    PyObject* tuple = PyTuple_New(1);
    PyTuple_SetItem(tuple, 0, list);
    PyObject* pRes = ptools.call("grav/py_scripts/test.py",
                                 "test_function", tuple);
    res = ptools.ltov(pRes);
    for ( int i = 0; i < res.size(); i++ ) {
        fprintf(stdout, "'%s'\n", res[i].c_str());
    }
    Py_DECREF(list);
}
