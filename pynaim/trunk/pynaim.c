#include <Python.h>
#include <naim/modutil.h>

extern conn_t *curconn;

static int cmd_pynaim(conn_t *c, const char *cmd, const char *arg) {
	if (strcasecmp(cmd, "PYEVAL") == 0)
		PyRun_SimpleString(arg);
	else if (strcasecmp(cmd, "PYLOAD") == 0) {
		FILE	*fp = fopen(arg, "r");

		PyRun_SimpleFile(fp, arg);
		fclose(fp);
	} else
		return(HOOK_CONTINUE);
	return(HOOK_STOP);
}

static PyObject* pynaim_echof(PyObject *self, PyObject *args) {
	const char *string;

	if (!PyArg_ParseTuple(args, "s:echof", &string))
		return(NULL);

	status_echof(curconn, string);
	Py_RETURN_NONE;
}

static PyMethodDef NaimModule[] = {
	{"echof", pynaim_echof, METH_VARARGS, "Does a status_echof call."},
	{NULL, NULL, 0, NULL},
};

int	naim_init(void *mod, const char *str) {
	Py_Initialize();
	Py_InitModule("naim", NaimModule);

	HOOK_ADD(getcmd, mod, cmd_pynaim, 100);

	return(MOD_REMAINLOADED);
}

int	naim_exit(void *mod, const char *str) {
	Py_Finalize();

	return(MOD_FINISHED);
}
