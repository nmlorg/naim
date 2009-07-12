#include <Python.h>
#include <naim/modutil.h>

typedef struct {
	PyObject_HEAD
	conn_t	*conn;
} _ConnectionObject;

static PyObject *_winname_get(_ConnectionObject *self, void *closure) {
	return PyString_FromString(self->conn->winname);
}

static PyGetSetDef _Connection_getset[] = {
	{"winname", (getter)_winname_get, NULL, "Connection name", NULL},
	{NULL},
};

static PyTypeObject _ConnectionType = {
	PyObject_HEAD_INIT(NULL)
	tp_name: "naim.types.Connection",
	tp_basicsize: sizeof(_ConnectionObject),
	tp_flags: Py_TPFLAGS_DEFAULT,
	tp_getset: _Connection_getset,
	tp_doc: "Connection Object",
	tp_new: PyType_GenericNew,
};

PyObject *pynaim_conn_wrap(conn_t *conn) {
	_ConnectionObject *connobj = PyObject_New(_ConnectionObject, &_ConnectionType);

	connobj->conn = conn;

	return (PyObject *)connobj;
}

void	pynaim_conn_init(void) {
	if (PyType_Ready(&_ConnectionType) < 0)
		return;

	Py_INCREF(&_ConnectionType);
	PyModule_AddObject(PyImport_AddModule("naim.types"), "Connection", (PyObject *)&_ConnectionType);
}
