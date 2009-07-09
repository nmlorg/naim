#include <Python.h>
#include <naim/modutil.h>

#include "_default_py.h"

extern conn_t *curconn;
void	*pynaim_mod = NULL;

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

static PyObject* pynaim_echo(PyObject *self, PyObject *args) {
	const char *string;

	if (!PyArg_ParseTuple(args, "s:echo", &string))
		return(NULL);

	echof(curconn, NULL, string);
	Py_RETURN_NONE;
}

static PyObject* pynaim_eval(PyObject *self, PyObject *args) {
	const char *string;

	if (!PyArg_ParseTuple(args, "s:eval", &string))
		return(NULL);

	secs_script_parse(string);
	Py_RETURN_NONE;
}

static PyMethodDef pynaimlib[] = {
	{"echo", pynaim_echo, METH_VARARGS,
	 "Display something on the screen with $-variable expansion."},
	{"eval", pynaim_eval, METH_VARARGS,
	 "Evaluate a command with $-variable substitution."},
	{NULL, NULL, 0, NULL},
};

int	pynaim_LTX_naim_init(void *mod, const char *str) {
	pynaim_mod = mod;

	Py_Initialize();
	PyObject *naimModule = Py_InitModule("naim", pynaimlib);
	pynaim_hooks_init(naimModule);
	PyRun_SimpleString(default_py);

	HOOK_ADD(getcmd, mod, cmd_pynaim, 100);

	return(MOD_REMAINLOADED);
}

int	pynaim_LTX_naim_exit(void *mod, const char *str) {
	HOOK_DEL(getcmd, mod, cmd_pynaim);

	Py_Finalize();

	pynaim_mod = NULL;

	return(MOD_FINISHED);
}
