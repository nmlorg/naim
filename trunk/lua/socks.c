/*  _ __   __ _ ___ __  __
** | '_ \ / _` |_ _|  \/  | naim
** | | | | | | || || |\/| | Copyright 1998-2006 Daniel Reed <n@ml.org>
** |_| |_|\__,_|___|_|  |_| ncurses-based chat client
*/
#include <stdarg.h>
#include "moon-int.h"

static int _nlua_socket_newsock(lua_State *L) {
	const char *name = lua_tostring(L, 1);
}

const struct luaL_reg naim_socketslib[] = {
	{ "newsock",		_nlua_socket_newsock },
};
