#include <Python.h>
#include <naim/modutil.h>

#include "_default_py.h"

extern conn_t *curconn;
void	pynaim_hooks_init(PyObject *parent);
void	*pynaim_mod = NULL;

static int pynaim_getcmd(conn_t *c, const char *cmd, const char *arg) {
	if (strcasecmp(cmd, "PYEVAL") == 0) {
        PyObject *result = PyRun_String(arg, Py_single_input, PyModule_GetDict(PyImport_AddModule("__main__")), NULL);

        if (result) {
            //echof(curconn, NULL, "Ran %s correctly", arg);
            PyObject *result_str = PyObject_Repr(result); // a string representation of result
            const char *str_str = PyString_AsString(result_str);
            echof(curconn, NULL, str_str);
            Py_DECREF(result);
            Py_DECREF(result_str);
        }
        else
            PyErr_Print();  // prints with naim.echo.  see default.py
        
    }
	else if (strcasecmp(cmd, "PYLOAD") == 0) {
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

			PyObject *module = PyImport_AddModule(modname);
			PyObject *module_dict = PyModule_GetDict(module);
			PyObject *exitfunc = PyDict_GetItemString(module_dict, "__exit__");
			if (exitfunc != NULL) {
				PyObject *arglist = Py_BuildValue("()");
				PyObject *result = PyObject_CallObject(exitfunc, arglist);
				Py_DECREF(arglist);
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
			fclose(fp);
		}
		if (freemodname)
			free(modname);
	} else
		return(HOOK_CONTINUE);
	return(HOOK_STOP);
}

static PyObject *pynaim_echo(PyObject *self, PyObject *args) {
	const char *string;

	if (!PyArg_ParseTuple(args, "s:echo", &string))
		return(NULL);

	echof(curconn, NULL, string);
	Py_RETURN_NONE;
}

static PyObject *pynaim_eval(PyObject *self, PyObject *args) {
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
	PyObject *naimmodule = Py_InitModule("naim", pynaimlib);
	pynaim_hooks_init(naimmodule);

	PyObject *eval_dict = PyDict_New();
	PyDict_SetItemString(eval_dict, "__builtins__", PyEval_GetBuiltins());
	PyObject *anonmodule = PyRun_String(default_py, Py_file_input, eval_dict, NULL);
	Py_DECREF(anonmodule);
	Py_DECREF(eval_dict);

	PyRun_SimpleString("import naim");

	HOOK_ADD(getcmd, mod, pynaim_getcmd, 100);

	return(MOD_REMAINLOADED);
}

int	pynaim_LTX_naim_exit(void *mod, const char *str) {
	HOOK_DEL(getcmd, mod, pynaim_getcmd);

	Py_Finalize();

	pynaim_mod = NULL;

	return(MOD_FINISHED);
}
