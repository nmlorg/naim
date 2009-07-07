#include <Python.h>
#include <naim/modutil.h>

static int cmd_pyeval(conn_t *c, const char *cmd, const char *arg) {
	if (strcasecmp(cmd, "PYEVAL") != 0)
		return(HOOK_CONTINUE);

	PyRun_SimpleString(arg);

	return(HOOK_STOP);
}

int	naim_init(void *mod, const char *str) {
	Py_Initialize();

	HOOK_ADD(getcmd, mod, cmd_pyeval, 100);

	return(MOD_REMAINLOADED);
}

int	naim_exit(void *mod, const char *str) {
	Py_Finalize();

	return(MOD_FINISHED);
}
