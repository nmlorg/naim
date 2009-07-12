#include <Python.h>
#include <naim/modutil.h>

extern conn_t *curconn;
extern void *pynaim_mod;
PyObject *pynaim_conn_wrap(conn_t *conn);

static int _run(void *userdata, PyObject *arglist) {
	PyObject *callable = (PyObject *)userdata,
		*result;
	long	ret = HOOK_CONTINUE;

	result = PyObject_CallObject(callable, arglist);
	Py_DECREF(arglist);
	if (result == NULL)
		PyErr_Print();
	else {
		if (result != Py_None)
			ret = PyInt_AsLong(result);
		Py_DECREF(result);
	}
	return(ret);
}

static int _periodic(void *userdata, void *dummy, time_t now) {
	PyObject *arglist = Py_BuildValue("(l)", (long)now);

	return _run(userdata, arglist);
}

static int _recvfrom(void *userdata, conn_t *conn, char **src, char **dst, unsigned char **message, int *len, int *flags) {
	PyObject *connobj = pynaim_conn_wrap(conn);
	PyObject *arglist = Py_BuildValue("(Osss#i)", connobj, *src, *dst, *message, *len, *flags);
	Py_DECREF(connobj);

	return _run(userdata, arglist);
}

static struct {
	const char *name;
	mod_hook_t func;
} _stubs[] = {
	{"periodic", (mod_hook_t)_periodic},
	{"recvfrom", (mod_hook_t)_recvfrom},
};

static mod_hook_t _getchain(const char *name) {
	int	i;

	for (i = 0; i < sizeof(_stubs)/sizeof(*_stubs); i++)
		if (strcasecmp(name, _stubs[i].name) == 0)
			return _stubs[i].func;

	PyErr_SetString(PyExc_NameError, "unknown chain");
	return(NULL);
}

static PyObject *_add(PyObject *self, PyObject *args) {
	const char *name;
	mod_hook_t func;
	int	weight;
	PyObject *callable;

	if (!PyArg_ParseTuple(args, "siO:hooks.add", &name, &weight, &callable))
		return(NULL);

	if ((func = _getchain(name)) == NULL)
		return(NULL);

	if (!PyCallable_Check(callable)) {
		PyErr_SetString(PyExc_TypeError, "parameter must be callable");
		return(NULL);
	}
	Py_INCREF(callable);

	hook_add(name, pynaim_mod, func, callable, weight, "run");

	Py_RETURN_NONE;
}

static PyObject *_del(PyObject *self, PyObject *args) {
	const char *name;
	mod_hook_t func;
	PyObject *callable;

	if (!PyArg_ParseTuple(args, "sO:hooks.add", &name, &callable))
		return(NULL);

	if ((func = _getchain(name)) == NULL)
		return(NULL);

	hook_del(name, pynaim_mod, func, callable);

	Py_DECREF(callable);

	Py_RETURN_NONE;
}

static PyMethodDef _hookslib[] = {
	{"add", _add, METH_VARARGS,
	 ""},
	{"delete", _del, METH_VARARGS,
	 ""},
	{NULL, NULL, 0, NULL},
};

void	pynaim_hooks_init(void) {
	PyObject *obj = Py_InitModule("naim.hooks", _hookslib);
	PyModule_AddObject(PyImport_AddModule("naim"), "hooks", obj);
	PyModule_AddIntConstant(obj, "HOOK_JUMP", HOOK_JUMP);
	PyModule_AddIntConstant(obj, "HOOK_STOP", HOOK_STOP);
	PyModule_AddIntConstant(obj, "HOOK_CONTINUE", HOOK_CONTINUE);
}
