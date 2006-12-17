/*  _ __   __ _ ___ __  __
** | '_ \ / _` |_ _|  \/  | naim
** | | | | | | || || |\/| | Copyright 1998-2006 Daniel Reed <n@ml.org>
** |_| |_|\__,_|___|_|  |_| ncurses-based chat client
*/

#include "moon-int.h"

#define NLUA_STRING_GET(ctype, varname) \
static int _nlua_ ## ctype ## _get_ ## varname(lua_State *L) { \
	ctype	*obj = (ctype *)lua_touserdata(L, 1); \
	\
	lua_pushstring(L, obj->varname?obj->varname:""); \
	return(1); \
}

static int _lua2conio(lua_State *L, int first, const char **args, const int argmax) {
	int	argc;

	for (argc = 0; (argc < argmax) && (lua_type(L, argc+first) != LUA_TNONE); argc++)
		args[argc] = lua_tostring(L, argc+first);

	return(argc);
}



void	nlua_hook_newconn(conn_t *conn) {
	_getsubtable("internal");
	_getitem("newconn");
	lua_pushstring(lua, conn->winname);
	lua_pushlightuserdata(lua, conn);
	if (lua_pcall(lua, 2, 0, 0))
		lua_pop(lua, 2);
}

void	nlua_hook_delconn(conn_t *conn) {
	_getsubtable("internal");
	_getitem("delconn");
	lua_pushstring(lua, conn->winname);
	if (lua_pcall(lua, 1, 0, 0))
		lua_pop(lua, 1);
}

void	_push_conn_t(lua_State *L, conn_t *conn) {
	_getsubtable("connections");
	lua_pushstring(L, conn->winname);
	lua_gettable(L, -2);
	lua_remove(L, -2);
}

static conn_t *_get_conn_t(lua_State *L, int index) {
	conn_t	*obj;

	lua_pushstring(L, "handle");
	lua_gettable(L, index);
	obj = (conn_t *)lua_touserdata(L, -1);
	lua_pop(L, 1);
	return(obj);
}

NLUA_STRING_GET(conn_t, sn);
NLUA_STRING_GET(conn_t, password);
NLUA_STRING_GET(conn_t, winname);
NLUA_STRING_GET(conn_t, server);
NLUA_STRING_GET(conn_t, profile);

static int l_conn_status_echo(lua_State *L) {
	/* lua_pushlightuserdata(L, void *p) */
	conn_t	*conn = _get_conn_t(L, 1);
	const char *s = lua_tostring(L, 2);

	if (!conn) {
		lua_pushstring(L, "conn was nil");
		return lua_error(L);
	}
	status_echof(conn, "%s", s);
	return(0);
}

#define NLUA_CONN_COMMAND(name) \
static int _nlua_conn_t_ ## name(lua_State *L) { \
	extern void ua_ ## name(conn_t *conn, int argc, const char **args); \
	conn_t *conn; \
	const char *args[UA_MAXPARMS]; \
	int	argc; \
	const char *error; \
	\
	if ((conn = _get_conn_t(L, 1)) == NULL) { \
		lua_pushstring(L, "No connection object; use naim.connections[CONN]:" #name " instead of naim.connections[CONN]." #name "."); \
		return(lua_error(L)); \
	} \
	argc = _lua2conio(L, 2, args, UA_MAXPARMS); \
	if ((error = ua_valid(#name, conn, argc)) == NULL) \
		ua_ ## name(conn, argc, args); \
	else { \
		lua_pushstring(L, error); \
		return(lua_error(L)); \
	} \
	return(0); \
}

#define CONN_COMMANDS \
	NN(echo) \
	NN(msg) \
	NN(say) \
	NN(me) \
	NN(ctcp) \
	NN(addbuddy) \
	NN(delbuddy) \
	NN(info) \
	NN(open) \
	NN(join) \
	NN(close) \
	NN(ignore) \
	NN(block) \
	NN(unblock) \
	NN(warn) \
	NN(setpriv) \
	NN(connect) \
	NN(server) \
	NN(disconnect) \

#define NN(x) NLUA_CONN_COMMAND(x)
CONN_COMMANDS
#undef NN

const struct luaL_reg naim_prototypes_connectionslib[] = {
	{ "get_sn",		_nlua_conn_t_get_sn },
	{ "get_password",	_nlua_conn_t_get_password },
	{ "get_winname",	_nlua_conn_t_get_winname },
	{ "get_server",		_nlua_conn_t_get_server },
	{ "get_profile",	_nlua_conn_t_get_profile },
	{ "status_echo",	l_conn_status_echo },
#define NN(x) { #x, _nlua_conn_t_ ## x },
CONN_COMMANDS
#undef NN
	{ NULL, 		NULL}
};




void	nlua_hook_newwin(buddywin_t *bwin) {
	_getsubtable("internal");
	_getitem("newwin");
	_push_conn_t(lua, bwin->conn);
	lua_pushstring(lua, bwin->winname);
	lua_pushlightuserdata(lua, bwin);
	if (lua_pcall(lua, 3, 0, 0))
		lua_pop(lua, 3);
}

void	nlua_hook_delwin(buddywin_t *bwin) {
	_getsubtable("internal");
	_getitem("delwin");
	_push_conn_t(lua, bwin->conn);
	lua_pushstring(lua, bwin->winname);
	if (lua_pcall(lua, 2, 0, 0))
		lua_pop(lua, 2);
}

static buddywin_t *_get_buddywin_t(lua_State *L, int index) {
	buddywin_t *obj;

	lua_pushstring(L, "handle");
	lua_gettable(L, index);
	obj = (buddywin_t *)lua_touserdata(L, -1);
	lua_pop(L, 1);
	return(obj);
}

NLUA_STRING_GET(buddywin_t, winname);

#define NLUA_BUDDYWIN_T_COMMAND(name) \
static int _nlua_buddywin_t_ ## name(lua_State *L) { \
	extern void ua_ ## name(conn_t *conn, int argc, const char **args); \
	buddywin_t *bwin; \
	const char *args[UA_MAXPARMS]; \
	int	argc; \
	const char *error; \
	\
	if ((bwin = _get_buddywin_t(L, 1)) == NULL) { \
		lua_pushstring(L, "No buddywin object; use naim.connections[CONN].buddies[BUDDY]:" #name " instead of naim.connections[CONN].buddies[BUDDY]." #name "."); \
		return(lua_error(L)); \
	} \
	args[0] = bwin->winname; \
	argc = _lua2conio(L, 2, args+1, UA_MAXPARMS-1)+1; \
	if ((error = ua_valid(#name, bwin->conn, argc)) == NULL) \
		ua_ ## name(bwin->conn, argc, args); \
	else { \
		lua_pushstring(L, error); \
		return(lua_error(L)); \
	} \
	return(0); \
}

#define BUDDYWIN_COMMANDS \
	NN(msg) \
	NN(say)

#define NN(x) NLUA_BUDDYWIN_T_COMMAND(x)
BUDDYWIN_COMMANDS
#undef NN

const struct luaL_reg naim_prototypes_windowslib[] = {
	{ "get_winname",	_nlua_buddywin_t_get_winname },
#define NN(x) { #x, _nlua_buddywin_t_ ## x },
BUDDYWIN_COMMANDS
#undef NN
	{ NULL,			NULL }
};




void	nlua_hook_newbuddy(buddylist_t *buddy) {
	_getsubtable("internal");
	_getitem("newbuddy");
	_push_conn_t(lua, buddy->conn);
	lua_pushstring(lua, USER_ACCOUNT(buddy));
	lua_pushlightuserdata(lua, buddy);
	if (lua_pcall(lua, 3, 0, 0))
		lua_pop(lua, 3);
}

void	nlua_hook_changebuddy(buddylist_t *buddy, const char *newaccount) {
	_getsubtable("internal");
	_getitem("changebuddy");
	_push_conn_t(lua, buddy->conn);
	lua_pushstring(lua, USER_ACCOUNT(buddy));
	lua_pushstring(lua, newaccount);
	if (lua_pcall(lua, 3, 0, 0))
		lua_pop(lua, 3);
}

void	nlua_hook_delbuddy(buddylist_t *buddy) {
	_getsubtable("internal");
	_getitem("delbuddy");
	_push_conn_t(lua, buddy->conn);
	lua_pushstring(lua, USER_ACCOUNT(buddy));
	if (lua_pcall(lua, 2, 0, 0))
		lua_pop(lua, 2);
}

const struct luaL_reg naim_prototypes_buddieslib[] = {
	{ NULL,			NULL }
};
