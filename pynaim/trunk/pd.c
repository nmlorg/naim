#include <Python.h>
#include <firetalk-int.h>
#include <naim/modutil.h>

extern conn_t *curconn;

static client_t _proto_create_handle(void) {
	return(NULL);
}

static void _proto_destroy_handle(client_t handle) {
}

static fte_t _proto_preselect(client_t handle, fd_set *read, fd_set *write, fd_set *except, int *n) {
	return(FE_SUCCESS);
}

static fte_t _proto_postselect(client_t handle, fd_set *read, fd_set *write, fd_set *except) {
	return(FE_SUCCESS);
}

static fte_t _proto_got_data(client_t handle, unsigned char *buffer, unsigned short *bufferpos) {
	return(FE_SUCCESS);
}

static fte_t _proto_got_data_connecting(client_t handle, unsigned char *buffer, unsigned short *bufferpos) {
	return(FE_SUCCESS);
}

static firetalk_protocol_t _proto_skeleton = {
	create_handle: _proto_create_handle,
	destroy_handle: _proto_destroy_handle,
	preselect: _proto_preselect,
	postselect: _proto_postselect,
	got_data: _proto_got_data,
	got_data_connecting: _proto_got_data_connecting,
};

static PyObject *_register_pd(PyObject *self, PyObject *arglist, PyObject *kwargs) {
	firetalk_protocol_t *proto = calloc(1, sizeof(*proto));
	char	*kwargs_names[] = {"strprotocol", "default_server", "default_port", NULL};

	memmove(proto, &_proto_skeleton, sizeof(*proto));

	if (!PyArg_ParseTupleAndKeywords(arglist, kwargs, "ssi|", kwargs_names,
		&(proto->strprotocol), &(proto->default_server), &(proto->default_port))) {
		return(NULL);
	}

	proto->strprotocol = strdup(proto->strprotocol);
	proto->default_server = strdup(proto->default_server);

	firetalk_register_protocol(proto);

	echof(curconn, NULL, "Registered %s.", proto->strprotocol);

	Py_RETURN_NONE;
}

static PyMethodDef _lib[] = {
	{"register", _register_pd, METH_VARARGS | METH_KEYWORDS,
	 ""},
	{NULL, NULL, 0, NULL},
};

void	pynaim_pd_init(void) {
	PyObject *obj = Py_InitModule("naim.pd", _lib);
	PyModule_AddObject(PyImport_AddModule("naim"), "pd", obj);
}
