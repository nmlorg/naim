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

mod_hook_t pynaim_getchain(const char *name) {
	int	i;

	for (i = 0; i < sizeof(hook_stubs)/sizeof(*hook_stubs); i++)
		if (strcasecmp(name, hook_stubs[i].name) == 0)
			return hook_stubs[i].func;

	PyErr_SetString(PyExc_NameError, "unknown chain");
	return(NULL);
}

static PyObject *pynaim_hooks_add(PyObject *self, PyObject *args) {
	const char *name;
	mod_hook_t func;
	int	weight;
	PyObject *callable;

	if (!PyArg_ParseTuple(args, "siO:hooks.add", &name, &weight, &callable))
		return(NULL);

	if ((func = pynaim_getchain(name)) == NULL)
		return(NULL);

	if (!PyCallable_Check(callable)) {
		PyErr_SetString(PyExc_TypeError, "parameter must be callable");
		return(NULL);
	}
	Py_INCREF(callable);

	hook_add(name, pynaim_mod, func, callable, weight, "pynaim_hooks_run");

	Py_RETURN_NONE;
}

static PyObject *pynaim_hooks_del(PyObject *self, PyObject *args) {
	const char *name;
	mod_hook_t func;
	PyObject *callable;

	if (!PyArg_ParseTuple(args, "sO:hooks.add", &name, &callable))
		return(NULL);

	if ((func = pynaim_getchain(name)) == NULL)
		return(NULL);

	hook_del(name, pynaim_mod, func, callable);

	Py_DECREF(callable);

	Py_RETURN_NONE;
}

static PyMethodDef pynaim_hookslib[] = {
	{"add", pynaim_hooks_add, METH_VARARGS,
	 ""},
	{"delete", pynaim_hooks_del, METH_VARARGS,
	 ""},
	{NULL, NULL, 0, NULL},
};

void	pynaim_hooks_init(PyObject *parent) {
	PyObject *obj = Py_InitModule("naim.hooks", pynaim_hookslib);
	PyModule_AddObject(parent, "hooks", obj);
	PyModule_AddIntConstant(obj, "HOOK_JUMP", HOOK_JUMP);
	PyModule_AddIntConstant(obj, "HOOK_STOP", HOOK_STOP);
	PyModule_AddIntConstant(obj, "HOOK_CONTINUE", HOOK_CONTINUE);
}
