/*  _ __   __ _ ___ __  __
** | '_ \ / _` |_ _|  \/  | naim
** | | | | | | || || |\/| | Copyright 1998-2006 Daniel Reed <n@ml.org>
** |_| |_|\__,_|___|_|  |_| ncurses-based chat client
*/

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "firetalk-int.h"
#include "moon-int.h"

struct firetalk_driver_connection_t {
	firetalk_driver_t pd;
};

static fte_t _nlua_pdcall(struct firetalk_driver_connection_t *c, const char *call, const char *signature, ...) {
	va_list	msg;
	int	i, args = 0, top = lua_gettop(lua);

	_get_global_ent(lua, "naim", "internal", "protos", c->pd.strprotocol, call, NULL);

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
				const char *str = va_arg(msg, const char *);
				uint32_t len = va_arg(msg, uint32_t);

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
				break;
			}
		  case HOOK_T_WRLSTRINGc: {
				const char **str = va_arg(msg, const char **);
				uint32_t *len = va_arg(msg, uint32_t *);

				lua_pushlstring(lua, *str, *len);
				break;
			}
		  case HOOK_T_WRUINT32c: {
				uint32_t *val = va_arg(msg, uint32_t *);

				lua_pushnumber(lua, *val);
				break;
			}
		  case HOOK_T_WRFLOATc: {
				double *val = va_arg(msg, double *);

				lua_pushnumber(lua, *val);
				break;
			}
		  default:
			lua_pushlightuserdata(lua, va_arg(msg, void *));
			break;
		}
	}
	va_end(msg);

	if (lua_pcall(lua, args, 0, 0) != 0) {
		extern conn_t *curconn;

		status_echof(curconn, "PD %s call %s run error: %s\n", c->pd.strprotocol, call, lua_tostring(lua, -1));
		lua_pop(lua, 1);
		return(HOOK_CONTINUE);
	}
	assert(lua_gettop(lua) == top);

	return(FE_SUCCESS);
}

static fte_t _nlua_pd_periodic(firetalk_connection_t *const conn) {
	return(FE_SUCCESS);
}

static fte_t _nlua_pd_preselect(struct firetalk_driver_connection_t *c, fd_set *read, fd_set *write, fd_set *except, int *n) {
	return(_nlua_pdcall(c, "preselect", HOOK_T_FDSET HOOK_T_FDSET HOOK_T_FDSET HOOK_T_WRUINT32, read, write, except, n));
}

static fte_t _nlua_pd_postselect(struct firetalk_driver_connection_t *c, fd_set *read, fd_set *write, fd_set *except) {
	return(_nlua_pdcall(c, "postselect", HOOK_T_FDSET HOOK_T_FDSET HOOK_T_FDSET, read, write, except));
}

static fte_t _nlua_pd_got_data(struct firetalk_driver_connection_t *c, firetalk_buffer_t *buffer) {
//	return(_nlua_pdcall(c, "got_data", HOOK_T
}

static fte_t _nlua_pd_got_data_connecting(struct firetalk_driver_connection_t *c, firetalk_buffer_t *buffer) {
//	return(_nlua_pdcall(c, "got_data_connecting", HOOK_T
}

static fte_t _nlua_pd_comparenicks(const char *const s1, const char *const s2) {
//	return(_nlua_pdcall(c, "comparenicks", HOOK_T_STRING HOOK_T_STRING, s1, s2));
}

static fte_t _nlua_pd_isprintable(const int key) {
//	return(_nlua_pdcall(c, "isprintable", HOOK_T_UINT32, key));
}

static fte_t _nlua_pd_disconnect(struct firetalk_driver_connection_t *c) {
	return(_nlua_pdcall(c, "disconnect", ""));
}

static fte_t _nlua_pd_disconnected(struct firetalk_driver_connection_t *c, const fte_t reason) {
	return(_nlua_pdcall(c, "disconnected", HOOK_T_STRING, firetalk_strerror(reason)));
}

static fte_t _nlua_pd_signon(struct firetalk_driver_connection_t *c, const char *const account) {
	return(_nlua_pdcall(c, "signon", HOOK_T_STRING, account));
}

static fte_t _nlua_pd_get_info(struct firetalk_driver_connection_t *c, const char *const account) {
	return(_nlua_pdcall(c, "get_info", HOOK_T_STRING, account));
}

static fte_t _nlua_pd_set_info(struct firetalk_driver_connection_t *c, const char *const text) {
	return(_nlua_pdcall(c, "set_info", HOOK_T_STRING, text));
}

static fte_t _nlua_pd_set_away(struct firetalk_driver_connection_t *c, const char *const text, const int isauto) {
	return(_nlua_pdcall(c, "set_away", HOOK_T_STRING HOOK_T_UINT32, text, isauto));
}

static fte_t _nlua_pd_set_nickname(struct firetalk_driver_connection_t *c, const char *const account) {
	return(_nlua_pdcall(c, "set_nickname", HOOK_T_STRING, account));
}

static fte_t _nlua_pd_set_password(struct firetalk_driver_connection_t *c, const char *const password, const char *const password2) {
	return(_nlua_pdcall(c, "set_password", HOOK_T_STRING, password));
}

static fte_t _nlua_pd_set_privacy(struct firetalk_driver_connection_t *c, const char *const flag) {
	return(_nlua_pdcall(c, "set_privacy", HOOK_T_STRING, flag));
}

static fte_t _nlua_pd_im_add_buddy(struct firetalk_driver_connection_t *c, const char *const account, const char *const group, const char *const friendly) {
	return(_nlua_pdcall(c, "im_add_buddy", HOOK_T_STRING HOOK_T_STRING HOOK_T_STRING, account, group, friendly));
}

static fte_t _nlua_pd_im_remove_buddy(struct firetalk_driver_connection_t *c, const char *const account, const char *const group) {
	return(_nlua_pdcall(c, "im_remove_buddy", HOOK_T_STRING HOOK_T_STRING, account, group));
}

static fte_t _nlua_pd_im_add_deny(struct firetalk_driver_connection_t *c, const char *const account) {
	return(_nlua_pdcall(c, "im_add_deny", HOOK_T_STRING, account));
}

static fte_t _nlua_pd_im_remove_deny(struct firetalk_driver_connection_t *c, const char *const account) {
	return(_nlua_pdcall(c, "im_remove_deny", HOOK_T_STRING, account));
}

static fte_t _nlua_pd_im_send_message(struct firetalk_driver_connection_t *c, const char *const account, const char *const text, const int isauto) {
	return(_nlua_pdcall(c, "im_send_message", HOOK_T_STRING HOOK_T_STRING HOOK_T_UINT32, account, text, isauto));
}

static fte_t _nlua_pd_im_send_action(struct firetalk_driver_connection_t *c, const char *const account, const char *const text, const int isauto) {
	return(_nlua_pdcall(c, "im_send_action", HOOK_T_STRING HOOK_T_STRING HOOK_T_UINT32, account, text, isauto));
}

static fte_t _nlua_pd_im_evil(struct firetalk_driver_connection_t *c, const char *const account) {
	return(_nlua_pdcall(c, "im_evil", HOOK_T_STRING, account));
}

static fte_t _nlua_pd_chat_join(struct firetalk_driver_connection_t *c, const char *const group) {
	return(_nlua_pdcall(c, "chat_join", HOOK_T_STRING, group));
}

static fte_t _nlua_pd_chat_part(struct firetalk_driver_connection_t *c, const char *const group) {
	return(_nlua_pdcall(c, "chat_part", HOOK_T_STRING, group));
}

static fte_t _nlua_pd_chat_invite(struct firetalk_driver_connection_t *c, const char *const group, const char *const account, const char *const text) {
	return(_nlua_pdcall(c, "chat_invite", HOOK_T_STRING HOOK_T_STRING HOOK_T_STRING, group, account, text));
}

static fte_t _nlua_pd_chat_set_topic(struct firetalk_driver_connection_t *c, const char *const group, const char *const text) {
	return(_nlua_pdcall(c, "chat_set_topic", HOOK_T_STRING HOOK_T_STRING, group, text));
}

static fte_t _nlua_pd_chat_op(struct firetalk_driver_connection_t *c, const char *const group, const char *const account) {
	return(_nlua_pdcall(c, "chat_op", HOOK_T_STRING HOOK_T_STRING, group, account));
}

static fte_t _nlua_pd_chat_deop(struct firetalk_driver_connection_t *c, const char *const group, const char *const account) {
	return(_nlua_pdcall(c, "chat_deop", HOOK_T_STRING HOOK_T_STRING, group, account));
}

static fte_t _nlua_pd_chat_kick(struct firetalk_driver_connection_t *c, const char *const group, const char *const account, const char *const text) {
	return(_nlua_pdcall(c, "chat_kick", HOOK_T_STRING HOOK_T_STRING HOOK_T_STRING, group, account, text));
}

static fte_t _nlua_pd_chat_send_message(struct firetalk_driver_connection_t *c, const char *const group, const char *const text, const int isauto) {
	return(_nlua_pdcall(c, "chat_send_message", HOOK_T_STRING HOOK_T_STRING HOOK_T_UINT32, group, text, isauto));
}

static fte_t _nlua_pd_chat_send_action(struct firetalk_driver_connection_t *c, const char *const group, const char *const text, const int isauto) {
	return(_nlua_pdcall(c, "chat_send_action", HOOK_T_STRING HOOK_T_STRING HOOK_T_UINT32, group, text, isauto));
}

static char *_nlua_pd_subcode_encode(struct firetalk_driver_connection_t *c, const char *const command, const char *const text) {
	return("mrow");
}

static const char *_nlua_pd_room_normalize(const char *const group) {
	return(group);
}

static struct firetalk_driver_connection_t *_nlua_pd_create_conn(struct firetalk_driver_cookie_t *cookie) {
//	_nlua_pdcall(&(cookie->pd), "create", HOOK
	return(NULL);
}

static void  _nlua_pd_destroy_conn(struct firetalk_driver_connection_t *c) {
	_nlua_pdcall(c, "destroy", "");
}

static const firetalk_driver_t firetalk_protocol_template = {
	periodic:		_nlua_pd_periodic,
	preselect:		_nlua_pd_preselect,
	postselect:		_nlua_pd_postselect,
	got_data:		_nlua_pd_got_data,
	got_data_connecting:	_nlua_pd_got_data_connecting,
	comparenicks:		_nlua_pd_comparenicks,
	isprintable:		_nlua_pd_isprintable,
	disconnect:		_nlua_pd_disconnect,
	disconnected:		_nlua_pd_disconnected,
	signon:			_nlua_pd_signon,
	get_info:		_nlua_pd_get_info,
	set_info:		_nlua_pd_set_info,
	set_away:		_nlua_pd_set_away,
	set_nickname:		_nlua_pd_set_nickname,
	set_password:		_nlua_pd_set_password,
	im_add_buddy:		_nlua_pd_im_add_buddy,
	im_remove_buddy:	_nlua_pd_im_remove_buddy,
	im_add_deny:		_nlua_pd_im_add_deny,
	im_remove_deny:		_nlua_pd_im_remove_deny,
	im_send_message:	_nlua_pd_im_send_message,
	im_send_action:		_nlua_pd_im_send_action,
	im_evil:		_nlua_pd_im_evil,
	chat_join:		_nlua_pd_chat_join,
	chat_part:		_nlua_pd_chat_part,
	chat_invite:		_nlua_pd_chat_invite,
	chat_set_topic:		_nlua_pd_chat_set_topic,
	chat_op:		_nlua_pd_chat_op,
	chat_deop:		_nlua_pd_chat_deop,
	chat_kick:		_nlua_pd_chat_kick,
	chat_send_message:	_nlua_pd_chat_send_message,
	chat_send_action:	_nlua_pd_chat_send_action,
//	subcode_send_request:	_nlua_pd_subcode_send_request,
//	subcode_send_reply:	_nlua_pd_subcode_send_reply,
	subcode_encode:		_nlua_pd_subcode_encode,
	set_privacy:		_nlua_pd_set_privacy,
	room_normalize:		_nlua_pd_room_normalize,
	create_conn:		_nlua_pd_create_conn,
	destroy_conn:		_nlua_pd_destroy_conn,
};

static int _nlua_create(lua_State *L) {
	firetalk_driver_t *pd;

	pd = malloc(sizeof(*pd));
	memmove(pd, &firetalk_protocol_template, sizeof(*pd));

	pd->strprotocol = strdup(lua_tostring(L, -4));
	pd->default_server = strdup(lua_tostring(L, -3));
	pd->default_port = lua_tonumber(L, -2);
	pd->default_buffersize = lua_tonumber(L, -1);
	pd->cookie = (struct firetalk_driver_cookie_t *)pd;

	firetalk_register_protocol(pd);

	return(0);
}

const struct luaL_reg naim_pdlib[] = {
	{ "create",	_nlua_create },
	{ NULL,		NULL }
};
