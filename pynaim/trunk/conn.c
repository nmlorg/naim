#include <Python.h>
#include <structmember.h>
#include <naim/modutil.h>

extern conn_t *curconn;
extern void *pynaim_mod;
void	pynaim_pausethreads(void);
void	pynaim_resumethreads(void);

typedef struct {
	PyObject_HEAD
	conn_t	*conn;
	PyObject *commands;
} _ConnectionObject;

static PyObject *_winname_get(_ConnectionObject *self, void *closure) {
	return PyString_FromString(self->conn->winname);
}

static PyGetSetDef _Connection_getset[] = {
	{"winname", (getter)_winname_get, NULL, "Connection name", NULL},
	{NULL},
};

static PyMemberDef _Connection_members[] = {
	{"commands", T_OBJECT_EX, offsetof(_ConnectionObject, commands), 0, "Commands"},
	{NULL},
};

static PyObject *_Connection_conio_stub(PyObject *self, PyObject *arglist) {
	_ConnectionObject *connobj = (_ConnectionObject *)self;
	PyObject *cfunc, *funcargs;

	if (!PyArg_ParseTuple(arglist, "OO", &cfunc, &funcargs))
		return(NULL);

	void	(*func)(conn_t *conn, int argc, const char **args) = PyCObject_AsVoidPtr(cfunc);

	const char *args[CONIO_MAXPARMS] = {0};
	int	i, funcargc = PyTuple_Size(funcargs);

	for (i = 0; i < funcargc; i++) {
		PyObject *funcarg = PyTuple_GetItem(funcargs, i);

		args[i] = PyString_AsString(funcarg);
	}

	func(connobj->conn, funcargc, args);

	Py_RETURN_NONE;
}

static PyMethodDef _Connection_methods[] = {
	{"_conio_stub", _Connection_conio_stub, METH_VARARGS,
	 "(Internal)"},
	{NULL},  /* Sentinel */
};

static PyTypeObject _ConnectionType = {
	PyObject_HEAD_INIT(NULL)
	tp_name: "naim.types.Connection",
	tp_basicsize: sizeof(_ConnectionObject),
	tp_flags: Py_TPFLAGS_DEFAULT,
	tp_getset: _Connection_getset,
	tp_doc: "Connection Object",
	tp_new: PyType_GenericNew,
	tp_members: _Connection_members,
	tp_methods: _Connection_methods,
};

typedef struct {
	const char *c;
	void	(*func)(conn_t *conn, int argc, char **args);
	const char *aliases[CONIO_MAXPARMS],
		*desc;
	const struct {
		const char required,
			type,
			*name;
	}	args[CONIO_MAXPARMS];
	int	minarg,
		maxarg,
		where;
} cmdar_t;

extern cmdar_t cmdar[];
extern int cmdc;

static void _register_cmdar() {
	PyObject *registerfunc = PyObject_GetAttrString(PyImport_AddModule("naim"), "_RegisterCommand");
	int	i;

	for (i = 0; i < cmdc; i++) {
		PyObject *cfunc = PyCObject_FromVoidPtr(cmdar[i].func, NULL);
		PyObject *result = PyObject_CallFunction(registerfunc, "(sOsii)", cmdar[i].c, cfunc, cmdar[i].desc, cmdar[i].minarg, cmdar[i].maxarg);
		Py_DECREF(cfunc);
		if (result == NULL)
			PyErr_Print();
		else
			Py_DECREF(result);
	}

	Py_DECREF(registerfunc);
}

static PyObject *_connections = NULL;

PyObject *pynaim_conn_wrap(conn_t *conn) {
	PyObject *result = PyDict_GetItemString(_connections, conn->winname);

	if (result != NULL) {
		Py_INCREF(result);
		return result;
	}

	_ConnectionObject *connobj = PyObject_New(_ConnectionObject, &_ConnectionType);

	connobj->conn = conn;

	PyObject *commandsfunc = PyObject_GetAttrString(PyImport_AddModule("naim.types"), "Commands");
	PyObject *commandsobj = PyObject_CallFunctionObjArgs(commandsfunc, connobj, NULL);
	Py_DECREF(commandsfunc);
	if (commandsobj == NULL)
		PyErr_Print();
	else
		connobj->commands = commandsobj;

	return (PyObject *)connobj;
}

static void _donewconn(conn_t *conn) {
	PyObject *connobj = pynaim_conn_wrap(conn);

	PyDict_SetItemString(_connections, conn->winname, connobj);
	Py_DECREF(connobj);
}

static int _newconn(conn_t *conn, const char *name, const char *protostr) {
	pynaim_pausethreads();

	_donewconn(conn);

	pynaim_resumethreads();

	return HOOK_CONTINUE;
}

static int _delconn(conn_t *conn, const char *name) {
	pynaim_pausethreads();

	PyDict_DelItemString(_connections, conn->winname);

	pynaim_resumethreads();

	return HOOK_CONTINUE;
}

void	pynaim_conn_init(void) {
	if (PyType_Ready(&_ConnectionType) < 0)
		return;

	Py_INCREF(&_ConnectionType);
	PyModule_AddObject(PyImport_AddModule("naim.types"), "Connection", (PyObject *)&_ConnectionType);

	_register_cmdar();

	_connections = PyDict_New();
	PyObject *connections_proxy = PyDictProxy_New(_connections);
	PyModule_AddObject(PyImport_AddModule("naim"), "connections", connections_proxy);

	conn_t	*conn = curconn;
	do {
		_donewconn(conn);
	} while ((conn = conn->next) != curconn);

	HOOK_ADD(newconn, pynaim_mod, _newconn, 50);
	HOOK_ADD(delconn, pynaim_mod, _delconn, 50);
}
