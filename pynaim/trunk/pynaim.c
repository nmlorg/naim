#include <Python.h>
#include <naim/modutil.h>

#include "_default_py.h"

extern conn_t *curconn;
void	pynaim_conn_init(void);
void	pynaim_hooks_init(void);
void	*pynaim_mod = NULL;

static PyThreadState *_pythreadstate_save = NULL;

static int _getcmd(conn_t *c, const char *cmd, const char *arg) {
	if (strcasecmp(cmd, "PYEVAL") == 0) {
		if (arg == NULL) {
			echof(c, cmd, "Command requires an argument.");
			return(HOOK_STOP);
		}

		PyEval_RestoreThread(_pythreadstate_save);

		PyObject *result = PyRun_String(arg, Py_single_input, PyModule_GetDict(PyImport_AddModule("__main__")), NULL);

		if (result == NULL)
			PyErr_Print();
		else
			Py_DECREF(result);

		_pythreadstate_save = PyEval_SaveThread();
	} else if (strcasecmp(cmd, "PYLOAD") == 0) {
		if (arg == NULL) {
			echof(c, cmd, "Command requires an argument.");
			return(HOOK_STOP);
		}

		char	*modname = strchr(arg, ' ');
		int	freemodname = 0;

		if (modname != NULL)
			*modname++ = 0;

		FILE	*fp = fopen(arg, "r");

		if (fp == NULL)
			echof(curconn, "PYLOAD", "%s", strerror(errno));
		else {
			if (modname == NULL) {
				if ((modname = strrchr(arg, '/')) == NULL)
					modname = arg;
				else
					modname++;
				modname = strdup(modname);
				freemodname = 1;
				char	*dot = strchr(modname, '.');
				if (dot != NULL)
					*dot = 0;
			}

			PyEval_RestoreThread(_pythreadstate_save);

			PyObject *module = PyImport_AddModule(modname);
			PyObject *module_dict = PyModule_GetDict(module);
			PyObject *exitfunc = PyDict_GetItemString(module_dict, "__exit__");
			if (exitfunc != NULL) {
				PyObject *result = PyObject_CallFunction(exitfunc, NULL);
				if (result == NULL)
					PyErr_Print();
				else
					Py_DECREF(result);
			}
			PyDict_Clear(module_dict);
			PyDict_SetItemString(module_dict, "__builtins__", PyEval_GetBuiltins());
			PyObject *result = PyRun_File(fp, arg, Py_file_input, module_dict, NULL);
			if (result == NULL)
				PyErr_Print();
			else {
				Py_DECREF(result);
				PyObject *mainmodule = PyImport_AddModule("__main__");
				PyObject_SetAttrString(mainmodule, modname, module);
			}

			_pythreadstate_save = PyEval_SaveThread();

			fclose(fp);
		}
		if (freemodname)
			free(modname);
	} else
		return(HOOK_CONTINUE);
	return(HOOK_STOP);
}

static PyObject *_echo(PyObject *self, PyObject *args) {
	const char *string;

	if (!PyArg_ParseTuple(args, "s:echo", &string))
		return(NULL);

	echof(curconn, NULL, string);
	Py_RETURN_NONE;
}

static PyObject *_eval(PyObject *self, PyObject *args) {
	const char *string;

	if (!PyArg_ParseTuple(args, "s:eval", &string))
		return(NULL);

	secs_script_parse(string);
	Py_RETURN_NONE;
}

static PyMethodDef pynaimlib[] = {
	{"echo", _echo, METH_VARARGS,
	 "Display something on the screen with $-variable expansion."},
	{"eval", _eval, METH_VARARGS,
	 "Evaluate a command with $-variable substitution."},
	{NULL, NULL, 0, NULL},
};

int	pynaim_LTX_naim_init(void *mod, const char *str) {
	pynaim_mod = mod;

	PyEval_InitThreads();
	Py_Initialize();
	PyObject *naimmodule = Py_InitModule("naim", pynaimlib);
	PyObject *naim_typesmodule = Py_InitModule("naim.types", NULL);
	PyModule_AddObject(naimmodule, "types", naim_typesmodule);

	pynaim_hooks_init();

	PyObject *eval_dict = PyDict_New();
	PyDict_SetItemString(eval_dict, "__builtins__", PyEval_GetBuiltins());
	PyObject *anonmodule = PyRun_String(default_py, Py_file_input, eval_dict, NULL);
	Py_DECREF(anonmodule);
	Py_DECREF(eval_dict);

	pynaim_conn_init();

	PyRun_SimpleString("import naim");

	HOOK_ADD(getcmd, mod, _getcmd, 100);

	_pythreadstate_save = PyEval_SaveThread();

	return(MOD_REMAINLOADED);
}

int	pynaim_LTX_naim_exit(void *mod, const char *str) {
	HOOK_DEL(getcmd, mod, _getcmd);

	PyEval_RestoreThread(_pythreadstate_save);

	Py_Finalize();

	pynaim_mod = NULL;

	return(MOD_FINISHED);
}
