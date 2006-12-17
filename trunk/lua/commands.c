/*  _ __   __ _ ___ __  __
** | '_ \ / _` |_ _|  \/  | naim
** | | | | | | || || |\/| | Copyright 1998-2006 Daniel Reed <n@ml.org>
** |_| |_|\__,_|___|_|  |_| ncurses-based chat client
*/

#include "moon-int.h"
#include "cmdar.h"

static int _lua2conio(lua_State *L, int first, const char **args, const int argmax) {
	int	argc;

	for (argc = 0; (argc < argmax) && (lua_type(L, argc+first) != LUA_TNONE); argc++)
		args[argc] = lua_tostring(L, argc+first);

	return(argc);
}

static int _table2conio(lua_State *L, int t, const char **args, const int argmax) {
	int	argc;

	lua_pushnil(L);
	for (argc = 0; (argc < argmax) && lua_next(L, t); argc++) {
		args[argc] = lua_tostring(L, -1);
		lua_pop(L, 1);
	}

	return(argc);
}

static conn_t *_get_conn_t(lua_State *L, int index) {
	conn_t	*obj;

	lua_pushstring(L, "handle");
	lua_gettable(L, index);
	obj = (conn_t *)lua_touserdata(L, -1);
	lua_pop(L, 1);
	return(obj);
}

#define NLUA_COMMAND(name) \
static int _nlua_command_ ## name(lua_State *L) { \
	extern void ua_ ## name(conn_t *conn, int argc, const char **args); \
	conn_t	*conn; \
	const char *args[UA_MAXPARMS]; \
	int	argc; \
	const char *error; \
	\
	if ((conn = _get_conn_t(L, 1)) == NULL) { \
		lua_pushstring(L, "No connection object; use naim.connections[CONN]:" #name " instead of naim.connections[CONN]." #name "."); \
		return(lua_error(L)); \
	} \
	if (lua_istable(L, 2)) \
		argc = _table2conio(L, 2, args, UA_MAXPARMS); \
	else if (lua_isstring(L, 2) || lua_isnumber(L, 2) || lua_isnil(L, 2)) \
		argc = _lua2conio(L, 2, args, UA_MAXPARMS); \
	else {\
		lua_pushstring(L, "Invalid argument passed to " #name "."); \
		return(lua_error(L)); \
	} \
	if ((error = ua_valid(#name, conn, argc)) == NULL) \
		ua_ ## name(conn, argc, args); \
	else { \
		lua_pushstring(L, error); \
		return(lua_error(L)); \
	} \
	return(0); \
}

#define UAFUNC3(x) NLUA_COMMAND(x)
NAIM_COMMANDS
#undef UAFUNC3

static const struct luaL_reg naim_commandslib[] = {
#define UAFUNC3(x) { #x, _nlua_command_ ## x },
NAIM_COMMANDS
#undef UAFUNC3
	{ NULL, 		NULL}
};

void	naim_commandsreg(lua_State *L) {
	int	i;

	assert(cmdc == sizeof(naim_commandslib)/sizeof(*naim_commandslib)-1);

	luaL_findtable(L, LUA_GLOBALSINDEX, "naim.commands", cmdc);

	for (i = 0; i < cmdc; i++) {
		assert(strcmp(cmdar[i].c, naim_commandslib[i].name) == 0);

		lua_pushstring(L, cmdar[i].c);
		lua_newtable(L);

		lua_pushstring(L, "func");
		lua_pushcfunction(L, naim_commandslib[i].func);
		lua_settable(L, -3);
		lua_pushstring(L, "min");
		lua_pushnumber(L, cmdar[i].minarg);
		lua_settable(L, -3);
		lua_pushstring(L, "max");
		lua_pushnumber(L, cmdar[i].maxarg);
		lua_settable(L, -3);
		lua_pushstring(L, "desc");
		lua_pushstring(L, cmdar[i].desc);
		lua_settable(L, -3);

		lua_settable(L, -3);
	}
}
