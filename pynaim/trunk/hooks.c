#include <Python.h>
#include <naim/modutil.h>

extern conn_t *curconn;
extern void *pynaim_mod;

static int pynaim_hooks_run(void *userdata, PyObject *arglist) {
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

static int pynaim_hooks_periodic(void *userdata, void *dummy, time_t now) {
	PyObject *arglist = Py_BuildValue("(l)", (long)now);

	return pynaim_hooks_run(userdata, arglist);
}

static int pynaim_hooks_recvfrom(void *userdata, conn_t *conn, char **src, char **dst, unsigned char **message, int *len, int *flags) {
	PyObject *arglist = Py_BuildValue("(isss#i)", 0, *src, *dst, *message, *len, *flags);

	return pynaim_hooks_run(userdata, arglist);
}

static struct {
	const char *name;
	mod_hook_t func;
} hook_stubs[] = {
	{"periodic", (mod_hook_t)pynaim_hooks_periodic},
	{"recvfrom", (mod_hook_t)pynaim_hooks_recvfrom},
};

static PyObject *pynaim_hooks_add(PyObject *self, PyObject *args) {
	const char *name;
	int	i, weight;
	PyObject *callable;

	if (!PyArg_ParseTuple(args, "siO:hooks.add", &name, &weight, &callable))
		return(NULL);

	for (i = 0; i < sizeof(hook_stubs)/sizeof(*hook_stubs); i++)
		if (strcasecmp(name, hook_stubs[i].name) == 0)
			break;

	if (i == sizeof(hook_stubs)/sizeof(*hook_stubs)) {
		PyErr_SetString(PyExc_NameError, "unknown chain");
		return(NULL);
	}

	if (!PyCallable_Check(callable)) {
		PyErr_SetString(PyExc_TypeError, "parameter must be callable");
		return(NULL);
	}
	Py_INCREF(callable);

	hook_add(name, pynaim_mod, hook_stubs[i].func, callable, weight, "pynaim_hooks_run");

	Py_RETURN_NONE;
}

static PyMethodDef pynaim_hookslib[] = {
	{"add", pynaim_hooks_add, METH_VARARGS,
	 ""},
	{NULL, NULL, 0, NULL},
};

void	pynaim_hooks_init() {
	Py_InitModule("naim.hooks", pynaim_hookslib);
}
