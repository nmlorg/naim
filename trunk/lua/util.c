/*  _ __   __ _ ___ __  __
** | '_ \ / _` |_ _|  \/  | naim
** | | | | | | || || |\/| | Copyright 1998-2006 Daniel Reed <n@ml.org>
** |_| |_|\__,_|___|_|  |_| ncurses-based chat client
*/
#include <stdarg.h>
#include "moon-int.h"

void	_get_entv(lua_State *L, const char *name, va_list msg) {
	while (name != NULL) {
		name = va_arg(msg, const char *);
	}
}

void	_get_ent(lua_State *L, const char *name, ...) {
	va_list	msg;

	va_start(msg, name);
	_get_entv(L, name, msg);
	va_end(msg);
}
