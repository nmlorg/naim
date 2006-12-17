/*  _ __   __ _ ___ __  __
** | '_ \ / _` |_ _|  \/  | naim
** | | | | | | || || |\/| | Copyright 1998-2006 Daniel Reed <n@ml.org>
** | | | | |_| || || |  | | moon.c Copyright 2006 Joshua Wise <joshua@joshuawise.com>
** |_| |_|\__,_|___|_|  |_| ncurses-based chat client
*/

#include "moon-int.h"

extern conn_t *curconn;

int	nlua_script_parse(const char *script) {
	if (luaL_loadstring(lua, script) != 0) {
		status_echof(curconn, "Parse error: \"%s\"", lua_tostring(lua, -1));
		lua_pop(lua, 1);		/* Error message that we don't care about. */
		return(0);
	}
	
	if (lua_pcall(lua, 0, 0, 0) != 0) {
		status_echof(curconn, "Lua function returned an error: \"%s\"", lua_tostring(lua, -1));
		lua_pop(lua, 1);
	}

	return(1);
}

char	*nlua_expand(const char *script) {
	char	*result;

	_get_global_ent(lua, "naim", "internal", "expandstring", NULL);
//	_getsubtable("internal");
//	_getitem("expandstring");
	lua_pushstring(lua, script);
	lua_pcall(lua, 1, 1, 0);		/* Feed the error message to the caller if there is one */
	result = strdup(lua_tostring(lua, -1));
	if (result != NULL)
		_garbage_add(result);
	lua_pop(lua, 1);
	return(result);
}

int	nlua_luacmd(conn_t *conn, char *cmd, char *arg) {
	char	*lcmd;

	_get_global_ent(lua, "naim.call", NULL);
//	_getmaintable();			// { naim }
//	_getitem("call");			// { naim.call }
	if (!lua_isfunction(lua, -1)) {
		static int complained = 0;

		lua_pop(lua, 1);		// {}
		if (!complained && (conn != NULL)) {
			complained++;
			status_echof(conn, "naim.call is no longer a function. This is a bug in a user script.");
		}
		return(0);
	}

	lcmd = strdup(cmd);
	{
		char	*p;

		for (p = lcmd; *p; p++)
			*p = tolower(*p);
	}
	_get_global_ent(lua, "naim", "commands", lcmd, NULL);
//	_getmaintable();			// { naim, naim.call }
//	_getitem("commands");			// { naim.commands, naim.call }
//	lua_pushstring(lua, lcmd);		// { CMD, naim.commands, naim.call }
	free(lcmd);
//	lua_gettable(lua, -2);			// { naim.commands[CMD], naim.commands, naim.call }
//	lua_remove(lua, -2);			// { naim.commands[CMD], naim.call }
	if (lua_isnil(lua, -1)) {
		lua_pop(lua, 2);		// {}
		return(0);
	}
	if (!lua_istable(lua, -1)) {
		lua_pop(lua, 2);		// {}
		if (conn != NULL)
			status_echof(conn, "naim.commands.%s is not a function table. This is a bug in a user script.", cmd);
		return(0);
	}
	_push_conn_t(lua, conn);		// { CONN, naim.commands[CMD], naim.call }
	lua_pushstring(lua, arg);		// { ARG, CONN, naim.commands[CMD], naim.call }
	if (lua_pcall(lua, 3, 0, 0) != 0) {	// {}
		if (conn != NULL)
			status_echof(conn, "Lua function \"%s\" returned an error: \"%s\"", cmd, lua_tostring(lua, -1));
		return(0);
	}
	return(1);
}
