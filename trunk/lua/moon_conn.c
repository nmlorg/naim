/*  _ __   __ _ ___ __  __
** | '_ \ / _` |_ _|  \/  | naim
** | | | | | | || || |\/| | Copyright 1998-2006 Daniel Reed <n@ml.org>
** | | | | |_| || || |  | | moon_conn.c Copyright 2006 Joshua Wise <joshua@joshuawise.com>
** |_| |_|\__,_|___|_|  |_| ncurses-based chat client
*/

#include <naim/naim.h>
#include <naim/modutil.h>
#include "naim-int.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "moon-int.h"

#define NLUA_STRING_GET(ctype, varname) \
static int _nlua_ ## ctype ## _get_ ## varname(lua_State *L) { \
	ctype	*obj = _get_ ## ctype(L, 1); \
	\
	lua_pushstring(L, obj->varname?obj->varname:""); \
	return(1); \
}




void nlua_hook_newconn(conn_t *conn)
{
	_getsubtable("internal");
	_getitem("newConn");
	lua_pushstring(lua, conn->winname);
	lua_pushlightuserdata(lua, conn);
	if (lua_pcall(lua, 2, 0, 0))
		lua_pop(lua, 2);
}

void nlua_hook_delconn(conn_t *conn)
{
	_getsubtable("internal");
	_getitem("delConn");
	lua_pushstring(lua, conn->winname);
	if (lua_pcall(lua, 1, 0, 0))
		lua_pop(lua, 1);
}

void _push_conn_t(lua_State *L, conn_t *conn) {
	_getsubtable("connections");
	lua_pushstring(lua, conn->winname);
	lua_gettable(lua, -2);
	lua_remove(lua, -2);
}

static conn_t *_get_conn_t(lua_State *L, int index) {
	return((conn_t *)lua_touserdata(L, index));
}

NLUA_STRING_GET(conn_t, sn);
NLUA_STRING_GET(conn_t, password);
NLUA_STRING_GET(conn_t, winname);
NLUA_STRING_GET(conn_t, server);
NLUA_STRING_GET(conn_t, profile);

static int l_conn_status_echo(lua_State *L)
{
	/* lua_pushlightuserdata(L, void *p) */
	conn_t *conn = _get_conn_t(L, 1);
	const char *s = lua_tostring(L, 2);
	
	if (!conn)
	{
		lua_pushstring(L, "conn was nil");
		return lua_error(L);
	}
	status_echof(conn, "%s", s);
	return 0;
}

static int l_conn_echo(lua_State *L) {
	conn_t *conn = _get_conn_t(L, 1);
	const char *s = lua_tostring(L, 2);
	
	if (conn == NULL) {
		lua_pushstring(L, "conn was nil");
		return(lua_error(L));
	}
	echof(conn, NULL, "%s", s);
	return(0);
}

typedef struct {
	int	argc;
	const char *args[CONIO_MAXPARMS];
} _lua2conio_ret;

static _lua2conio_ret *_lua2conio(lua_State *L, int first) {
	static _lua2conio_ret ret;

	for (ret.argc = 0; (ret.argc < CONIO_MAXPARMS) && (lua_type(L, ret.argc+first) != LUA_TNONE); ret.argc++)
		ret.args[ret.argc] = lua_tostring(L, ret.argc+first);

	return(&ret);
}

static int l_conn_msg(lua_State *L) {
	extern void conio_msg(conn_t *conn, int argc, const char **args);
	conn_t *conn = _get_conn_t(L, 1);
	_lua2conio_ret *ret = _lua2conio(L, 2);
	const char *error;

	if ((error = conio_valid("msg", conn, ret->argc)) == NULL)
		conio_msg(conn, ret->argc, ret->args);
	else {
		lua_pushstring(L, error);
		return(lua_error(L));
	}
	return(0);
}

const struct luaL_reg naimprototypeconnlib[] = {
	{ "get_sn",		_nlua_conn_t_get_sn },
	{ "get_password",	_nlua_conn_t_get_password },
	{ "get_winname",	_nlua_conn_t_get_winname },
	{ "get_server",		_nlua_conn_t_get_server },
	{ "get_profile",	_nlua_conn_t_get_profile },
	{ "status_echo",	l_conn_status_echo },
	{ "echo",		l_conn_echo },
	{ "msg",		l_conn_msg },
	{ NULL, 		NULL}
};




void	nlua_hook_newwin(conn_t *conn, buddywin_t *bwin) {
	_getsubtable("internal");
	_getitem("newwin");
	_push_conn_t(lua, conn);
	lua_pushstring(lua, bwin->winname);
	lua_pushlightuserdata(lua, bwin);
	if (lua_pcall(lua, 3, 0, 0))
		lua_pop(lua, 3);
}

void	nlua_hook_delwin(conn_t *conn, buddywin_t *bwin) {
	_getsubtable("internal");
	_getitem("delwin");
	_push_conn_t(lua, conn);
	lua_pushstring(lua, bwin->winname);
	if (lua_pcall(lua, 2, 0, 0))
		lua_pop(lua, 2);
}

static buddywin_t *_get_buddywin_t(lua_State *L, int index) {
	return((buddywin_t *)lua_touserdata(L, index));
}

NLUA_STRING_GET(buddywin_t, winname);

static int _nlua_buddywin_t_msg(lua_State *L) {
	extern void conio_msg(conn_t *conn, int argc, const char **args);
	conn_t *conn = _get_conn_t(L, 1);
	buddywin_t *bwin = _get_buddywin_t(L, 2);
	_lua2conio_ret *ret = _lua2conio(L, 3);
	const char *error;

	if ((error = conio_valid("msg", conn, ret->argc)) == NULL)
		conio_msg(conn, ret->argc, ret->args);
	else {
		lua_pushstring(L, error);
		return(lua_error(L));
	}
	return(0);
}

const struct luaL_reg naimprototypewindows[] = {
	{ "get_winname",	_nlua_buddywin_t_get_winname },
	{ "msg",		_nlua_buddywin_t_msg },
	{ NULL,			NULL }
};




void	nlua_hook_newbuddy(conn_t *conn, buddylist_t *buddy) {
	_getsubtable("internal");
	_getitem("newbuddy");
	_push_conn_t(lua, conn);
	lua_pushstring(lua, USER_ACCOUNT(buddy));
	lua_pushlightuserdata(lua, buddy);
	if (lua_pcall(lua, 3, 0, 0))
		lua_pop(lua, 3);
}

void	nlua_hook_changebuddy(conn_t *conn, buddylist_t *buddy, const char *newaccount) {
	_getsubtable("internal");
	_getitem("changebuddy");
	_push_conn_t(lua, conn);
	lua_pushstring(lua, USER_ACCOUNT(buddy));
	lua_pushstring(lua, newaccount);
	if (lua_pcall(lua, 3, 0, 0))
		lua_pop(lua, 3);
}

void	nlua_hook_delbuddy(conn_t *conn, buddylist_t *buddy) {
	_getsubtable("internal");
	_getitem("delbuddy");
	_push_conn_t(lua, conn);
	lua_pushstring(lua, USER_ACCOUNT(buddy));
	if (lua_pcall(lua, 2, 0, 0))
		lua_pop(lua, 2);
}

const struct luaL_reg naimprototypebuddies[] = {
	{ NULL,			NULL }
};
