
#include "PythonTools.h"

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

PyObject* PythonTools::call( std::string _mod, std::string _func,
                             PyObject* args )
{
    PyObject *module, *func, *name, *result;
    name = PyString_FromString(_mod.c_str());
    module = PyImport_Import(name);
    Py_DECREF(name);

    if ( module == NULL ) {
        PyErr_Print();
        fprintf(stderr, "Failed to load module \"%s\"\n", _mod.c_str());
        return NULL;
    }

    func = PyObject_GetAttrString(module, _func.c_str());
    if ( func == NULL ) {
        PyErr_Print();
        fprintf(stderr, "Failed to load function \"%s\"\n", _func.c_str());
        Py_DECREF(module);
        return NULL;
    }
    if ( ! PyCallable_Check(func) ) {
        PyErr_Print();
        fprintf(stderr, "Attribute \"%s\" is not callable.\n", _func.c_str());
        Py_DECREF(module);
        Py_DECREF(func);
        return NULL;
    }

    // The real work:
    result = PyObject_CallObject(func, args);

    if ( result == NULL ) {
        PyErr_Print();
        fprintf(stderr, "Call failed.\n");
        Py_DECREF(module);
        Py_DECREF(func);
        return NULL;
    }
    return result;
}

#include <iostream>
/* Just a test... */
int main(int argc, char *argv[])
{
    PythonTools ptools = PythonTools();
    std::vector<std::string> v = std::vector<std::string>();
    v.push_back("foobar");
    v.push_back("oh noes");
    PyObject* list = ptools.vtol(v);
    std::vector<std::string> res = ptools.ltov(list);
    std::cout << "(if you can see this.. it worked.)\n";
    for ( int i = 0; i < res.size(); i++ ) {
        fprintf(stdout, "'%s'\n", res[i].c_str());
    }
    std::cout << "(did it work?)\n";
    std::cout << "(trying a function call now)\n";
    PyObject* pRes = ptools.call("grav.py_scripts.test", "test_function", list);
    std::cout << "(converting the output)\n";
    res = ptools.ltov(list);
    std::cout << "(printing the output)\n";
    for ( int i = 0; i < res.size(); i++ ) {
        fprintf(stdout, "'%s'\n", res[i].c_str());
    }
    std::cout << "(done)\n";
    Py_DECREF(list);
}
