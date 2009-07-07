#include <Python.h>
#include <naim/modutil.h>

extern conn_t *curconn;
extern void *pynaim_mod;

static int pynaim_hooks_run(void *userdata, ...) {
	PyObject *callable = (PyObject *)userdata,
		*arglist = Py_BuildValue("(i)", 0),
		*result;

	result = PyObject_CallObject(callable, arglist);
	Py_DECREF(arglist);
	Py_DECREF(result);
	return(HOOK_CONTINUE);
}

static PyObject *pynaim_hooks_add(PyObject *self, PyObject *args) {
	const char *name;
	int	weight;
	PyObject *callable;

	if (!PyArg_ParseTuple(args, "siO:hooks.add", &name, &weight, &callable))
		return(NULL);

	if (!PyCallable_Check(callable)) {
		PyErr_SetString(PyExc_TypeError, "parameter must be callable");
		return(NULL);
	}
	Py_INCREF(callable);

	hook_add(name, pynaim_mod, pynaim_hooks_run, callable, weight, "pynaim_hooks_run");

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
