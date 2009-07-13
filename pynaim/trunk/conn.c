#include <Python.h>
#include <naim/modutil.h>

extern conn_t *curconn;
extern void *pynaim_mod;

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

static PyObject *_Connection_msg(PyObject *self, PyObject *args) {
    const char *dst;
    const char *msg;
    _ConnectionObject* rawself = (_ConnectionObject*)self;

    if (!PyArg_ParseTuple(args, "ss:msg", &dst, &msg))
        return(NULL);

    char *tosend[] = {dst, msg};
    conio_msg(rawself->conn, 2, tosend);
    Py_RETURN_NONE;

}

static PyMethodDef _Connection_methods[] = {
    {"msg", _Connection_msg, METH_VARARGS,
        "Send a message on a connection"
    },
    {NULL}  /* Sentinel */
};

static PyTypeObject _ConnectionType = {
	PyObject_HEAD_INIT(NULL)
	tp_name: "naim.types.Connection",
	tp_basicsize: sizeof(_ConnectionObject),
	tp_flags: Py_TPFLAGS_DEFAULT,
	tp_getset: _Connection_getset,
	tp_doc: "Connection Object",
	tp_new: PyType_GenericNew,
    tp_methods: _Connection_methods
};

static PyObject *_connections = NULL;

PyObject *pynaim_conn_wrap(conn_t *conn) {
	PyObject *result = PyDict_GetItemString(_connections, conn->winname);

	if (result != NULL) {
		Py_INCREF(result);
		return result;
	}

	_ConnectionObject *connobj = PyObject_New(_ConnectionObject, &_ConnectionType);

	connobj->conn = conn;

	return (PyObject *)connobj;
}

static int _newconn(conn_t *conn, const char *name, const char *protostr) {
	PyObject *connobj = pynaim_conn_wrap(conn);

	PyDict_SetItemString(_connections, conn->winname, connobj);
	Py_DECREF(connobj);

	return HOOK_CONTINUE;
}

static int _delconn(conn_t *conn, const char *name) {
	PyDict_DelItemString(_connections, conn->winname);

	return HOOK_CONTINUE;
}

void	pynaim_conn_init(void) {
	if (PyType_Ready(&_ConnectionType) < 0)
		return;

	Py_INCREF(&_ConnectionType);
	PyModule_AddObject(PyImport_AddModule("naim.types"), "Connection", (PyObject *)&_ConnectionType);

	_connections = PyDict_New();
	PyObject *connections_proxy = PyDictProxy_New(_connections);
	PyModule_AddObject(PyImport_AddModule("naim"), "connections", connections_proxy);

	conn_t	*conn = curconn;
	do {
		_newconn(conn, NULL, NULL);
	} while ((conn = conn->next) != curconn);

	HOOK_ADD(newconn, pynaim_mod, _newconn, 50);
	HOOK_ADD(delconn, pynaim_mod, _delconn, 50);
}
