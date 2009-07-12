#include <Python.h>

typedef struct {
    PyObject_HEAD
        /* Type-specific fields go here. */
        /* TODO(?) add the naim connection specific fields here? */
    int my_val;
} naim_ConnectionObject;

PyObject* noddy_getattr(PyObject *o, PyObject *attr_name) {
    PyObject* cats = PyString_FromString("cats");
    if (PyObject_Compare(cats, attr_name) == 0) {
        Py_DECREF(cats);
        return Py_BuildValue("s", "mrow");
    } else {
        Py_DECREF(cats);
        PyErr_SetString(PyExc_AttributeError,"Unknown attribute");
        return NULL;
    }
}

static PyTypeObject naim_ConnectionType = {
    PyObject_HEAD_INIT(NULL)
    tp_name: "naim.Connection",
    tp_basicsize: sizeof(naim_ConnectionObject),
    tp_getattro: noddy_getattr, // TODO(?) switch to using tp_getset
    tp_flags: Py_TPFLAGS_DEFAULT,
    tp_doc: "Connection Object",
};



static PyMethodDef noddy_methods[] = {
        {NULL}  /* Sentinel */
};

void pynaim_conn_init(PyObject *parent) {

    naim_ConnectionType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&naim_ConnectionType) < 0)
        return;

    //TODO(?) write an object that actually implements this ConnectionType

    Py_INCREF(&naim_ConnectionType);
    PyModule_AddObject(parent, "Connection", (PyObject *)&naim_ConnectionType);

}
