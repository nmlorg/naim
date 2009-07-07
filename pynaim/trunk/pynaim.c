#include <Python.h>
#include <naim/modutil.h>

extern conn_t *curconn;

static int cmd_pyeval(conn_t *c, const char *cmd, const char *arg) {
	if (strcasecmp(cmd, "PYEVAL") != 0)
		return(HOOK_CONTINUE);

	PyRun_SimpleString(arg);

	return(HOOK_STOP);
}

static PyObject* pynaim_echof(PyObject *self, PyObject *args) {
	const char *string;

	if (!PyArg_ParseTuple(args, "s:echof", &string))
		return(NULL);

	if (curconn) {
		status_echof(curconn, string);
		return(Py_BuildValue("i", 0));
	} else
		return(NULL);
}

static PyMethodDef NaimModule[] = {
	{"echof", pynaim_echof, METH_VARARGS, "Does a status_echof call."},
	{NULL, NULL, 0, NULL},
};

int	naim_init(void *mod, const char *str) {
	Py_Initialize();
	Py_InitModule("naim", NaimModule);

	HOOK_ADD(getcmd, mod, cmd_pyeval, 100);

	return(MOD_REMAINLOADED);
}

int	naim_exit(void *mod, const char *str) {
	Py_Finalize();

	return(MOD_FINISHED);
}
