/*  _ __   __ _ ___ __  __
** | '_ \ / _` |_ _|  \/  | naim
** | | | | | | || || |\/| | Copyright 1998-2006 Daniel Reed <n@ml.org>
** | | | | |_| || || |  | | moon.c Copyright 2006 Joshua Wise <joshua@joshuawise.com>
** |_| |_|\__,_|___|_|  |_| ncurses-based chat client
*/

#include "moon-int.h"
#include "default_lua.h"

lua_State *lua = NULL;
extern conn_t *curconn;
extern void (*script_client_cmdhandler)(const char *);

/* Note: This replaces stuff that was previously handled by SECS.
 * Notable changes --
 *  * Previously, SECS allowed *all* characters in variable names. Now, nLua
 *    requires variable names to match the regexp "[a-zA-Z0-9:_]+".  nLua may
 *    not barf if you don't satisfy this, but it might not always expand
 *    properly. 
 *  * Previously, the scripting engine was called SECS. It is now
 *    called Lua, and is interfaced by Moon. Lua is Portuguese for Moon.
 */

static int _nlua_debug(lua_State *L) {
	const char *s = lua_tostring(L, 1);

	status_echof(curconn, "%s", s);
	return(0);
}

static int _nlua_curconn(lua_State *L) {
	_push_conn_t(L, curconn);
	return(1);
}

static int _nlua_conio(lua_State *L) {
	const char *s = lua_tostring(L, 1);
	
	if (s == NULL)
		return(luaL_error(L, "l_conio: string was nil"));
	script_client_cmdhandler(s);
	return(0);
}

static int _nlua_echo(lua_State *L) {
	echof(curconn, NULL, "%s\n", lua_tostring(L, 1));
	return(0);
}

static const struct luaL_Reg naimlib[] = {
	{ "debug",	_nlua_debug },
	{ "curconn",	_nlua_curconn },
	{ "conio",	_nlua_conio },
	{ "echo",	_nlua_echo },
	{ NULL,		NULL } /* sentinel */
};

static const char *grabword(char *str) {
	int	inquote = 0;
	char	*start;

	while (isspace(*str))
		str++;
	start = str;

	while ((*str != 0) && (inquote || !isspace(*str))) {
		if (*str == '"') {
			memmove(str, str+1, strlen(str+1)+1);
			inquote = !inquote;
			continue;
		}
		str++;
	}

	*str = 0;

	return(start);
}

static int _nlua_pullword(lua_State *L) {
	const char *string = lua_tostring(L, 1), *car, *cdr;
	char	*copy;

	if (string == NULL)
		return(luaL_error(L, "_nlua_firstwhite: string was nil"));
	copy = malloc(strlen(string)+2);
	strncpy(copy, string, strlen(string)+2);
	car = grabword(copy);
	lua_pushstring(L, car);
	cdr = car + strlen(car)+1;
	while (isspace(*cdr))
		cdr++;
	if (*cdr != 0)
		lua_pushstring(L, cdr);
	else
		lua_pushnil(L);
	free(copy);
	return(2);
}

static const struct luaL_reg naim_internallib[] = {
	{ "pullword",	_nlua_pullword },
	{ NULL,		NULL } /* sentinel */
};

static int _nlua_hook(void *userdata, const char *signature, ...) {
	va_list	msg;
	int	ref = (int)userdata, i, args = 0, recoverable = 0;

	if (luaL_findtable(lua, LUA_GLOBALSINDEX, "naim.internal.hooks", 1) != NULL)
		abort();
	lua_rawgeti(lua, -1, ref);
	lua_remove(lua, -2);

	va_start(msg, signature);
	for (i = 0; signature[i] != 0; i++) {
		args++;
		switch(signature[i]) {
		  case HOOK_T_CONNc:
			_push_conn_t(lua, va_arg(msg, conn_t *));
			break;
		  case HOOK_T_STRINGc:
			lua_pushstring(lua, va_arg(msg, const char *));
			break;
		  case HOOK_T_LSTRINGc: {
				uint32_t len = va_arg(msg, uint32_t);
				const char *str = va_arg(msg, const char *);

				lua_pushlstring(lua, str, len);
				break;
			}
		  case HOOK_T_UINT32c:
			lua_pushnumber(lua, va_arg(msg, uint32_t));
			break;
		  case HOOK_T_FLOATc:
			lua_pushnumber(lua, va_arg(msg, double));
			break;
		  case HOOK_T_WRSTRINGc: {
				const char **str = va_arg(msg, const char **);

				lua_pushstring(lua, *str);
				recoverable++;
				break;
			}
		  case HOOK_T_WRLSTRINGc: {
				uint32_t *len = va_arg(msg, uint32_t *);
				const char **str = va_arg(msg, const char **);

				lua_pushlstring(lua, *str, *len);
				recoverable++;
				break;
			}
		  case HOOK_T_WRUINT32c: {
				uint32_t *val = va_arg(msg, uint32_t *);

				lua_pushnumber(lua, *val);
				recoverable++;
				break;
			}
		  case HOOK_T_WRFLOATc: {
				double *val = va_arg(msg, double *);

				lua_pushnumber(lua, *val);
				recoverable++;
				break;
			}
		  default:
			lua_pushlightuserdata(lua, va_arg(msg, void *));
			break;
		}
	}
	va_end(msg);

	if (lua_pcall(lua, args, 1+recoverable, 0) != 0) {
		status_echof(curconn, "Chain %d run error: %s\n", ref, lua_tostring(lua, -1));
		lua_pop(lua, 1);
		return(HOOK_CONTINUE);
	}
	if (!lua_isnumber(lua, -1)) {
		lua_pop(lua, 1);
		return(HOOK_CONTINUE);
	}
	i = lua_tonumber(lua, -1);
	lua_pop(lua, 1);

	return(i);
}

static int _nlua_hooks_add(lua_State *L) {
	void	*mod = NULL;
	const char *chainname;
	int	weight,
		ref;

	chainname = luaL_checkstring(L, 1);
	luaL_checktype(L, 2, LUA_TFUNCTION);
	weight = luaL_checkint(L, 3);
	if (luaL_findtable(L, LUA_GLOBALSINDEX, "naim.internal.hooks", 1) != NULL)
		return(luaL_error(L, "Hooks table damaged"));
	lua_pushvalue(L, 2);
	ref = luaL_ref(L, -2); //You can retrieve an object referred by reference r by calling lua_rawgeti(L, t, r). 
	lua_pop(L, 2);

	HOOK_ADD2(chainname, mod, _nlua_hook, weight, (void *)ref);

	lua_pushlightuserdata(L, (void *)ref);	/* opaque reference */
	return(1);
}

static int _nlua_hooks_del(lua_State *L) {
	const char *chainname;
	int	ref;
	void	*mod = NULL;

	chainname = luaL_checkstring(L, 1);
	if (!lua_islightuserdata(L, 2))
		return(luaL_typerror(L, 1, "light userdata"));
	ref = (int)lua_touserdata(L, 2);

	HOOK_DEL2(chainname, mod, _nlua_hook, (void *)ref);

	if (luaL_findtable(L, LUA_GLOBALSINDEX, "naim.internal.hooks", 1) != NULL)
		return(luaL_error(L, "Hooks table damaged"));
	luaL_unref(L, -1, ref);
	lua_pop(L, 1);

	return(0);
}

static const struct luaL_reg naim_hookslib[] = {
	{ "add",	_nlua_hooks_add },
	{ "del",	_nlua_hooks_del },
	{ NULL,		NULL } /* sentinel */
};

static void _loadfunctions(void) {
	extern const struct luaL_reg naim_prototypes_connectionslib[],
		naim_prototypes_windowslib[],
		naim_prototypes_buddieslib[];
	extern void naim_commandsreg(lua_State *L);

	luaL_register(lua, "naim", naimlib);
	luaL_register(lua, "naim.internal", naim_internallib);
	luaL_register(lua, "naim.prototypes.connections", naim_prototypes_connectionslib);
	luaL_register(lua, "naim.prototypes.windows", naim_prototypes_windowslib);
	luaL_register(lua, "naim.prototypes.buddies", naim_prototypes_buddieslib);
	luaL_register(lua, "naim.hooks", naim_hookslib);
	naim_commandsreg(lua);
}

void	nlua_init(void) {
	lua = luaL_newstate();
	
	/* XXX: Do we need to set a panic function here? */
	lua_gc(lua, LUA_GCSTOP, 0);	/* Paul says we should stop the garbage collector while we bring in libraries. */
		luaL_openlibs(lua);
		_loadfunctions();		/* this creates global "naim" for default.lua */
	lua_gc(lua, LUA_GCRESTART, 0);
	
	if (luaL_loadstring(lua, default_lua) != 0) {
		printf("default.lua load error: %s\n", lua_tostring(lua, -1));
		abort();
	}
	if (lua_pcall(lua, 0, 0, 0) != 0) {
		printf("default.lua run error: %s\n", lua_tostring(lua, -1));
		abort();
	}
}

void	nlua_shutdown(void) {
	lua_close(lua);
}
