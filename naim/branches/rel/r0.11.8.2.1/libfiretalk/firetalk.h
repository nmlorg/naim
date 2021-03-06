/*
firetalk.h - FireTalk wrapper declarations
Copyright (C) 2000 Ian Gulliver

This program is free software; you can redistribute it and/or modify
it under the terms of version 2 of the GNU General Public License as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef _FIRETALK_H
#define _FIRETALK_H

#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <netinet/in.h>

#define _FC_USE_IPV6

#ifndef _HAVE_FIRETALK_T
typedef void *firetalk_t;
#define _HAVE_FIRETALK_T
#endif

/* enums */
enum firetalk_protocol {
#if 0
	FP_AIMTOC,
	FP_IRC,
	FP_LILY,
	FP_MAX /* tracking enum, don't use this */
#else
	FP_DUMMY
#endif
};

enum firetalk_callback {
	FC_CONNECTED,
		/* void *connection, void *clientstruct */
	FC_CONNECTFAILED,
		/* void *connection, void *clientstruct, int error, char *reason */
	FC_DOINIT,
		/* void *connection, void *clientstruct, char *nickname */
	FC_ERROR,
		/* void *connection, void *clientstruct, int error, char *roomoruser (room or user that error applies to, null if none) */
	FC_DISCONNECT,
		/* void *connection, void *clientstruct, int error */
	FC_SETIDLE,
		/* void *connection, void *clientstruct, long *idle */
	FC_EVILED,
		/* void *connection, void *clientstruct, int newevil, char *eviler */
	FC_NEWNICK,
		/* void *connection, void *clientstruct, char *nickname */
	FC_PASSCHANGED,
		/* void *connection, void *clientstruct */
	FC_NEEDPASS,
		/* void *connection, void *clientstruct, char *pass, int size */
	FC_PRESELECT,
		/* void *connection, void *clientstruct */
	FC_POSTSELECT,
		/* void *connection, void *clientstruct */
	FC_IM_IDLEINFO,
		/* void *connection, void *clientstruct, char *nickname, long idletime */
	FC_IM_EVILINFO,
		/* void *connection, void *clientstruct, char *nickname, long warnval */
	FC_IM_BUDDYADDED,
		/* void *connection, void *clientstruct, char *nickname, char *group, char *friendly */
	FC_IM_BUDDYREMOVED,
		/* void *connection, void *clientstruct, char *nickname */
	FC_IM_TYPINGINFO,
		/* void *connection, void *clientstruct, char *nickname, int typing */
	FC_IM_CAPABILITIES,
		/* void *connection, void *clientstruct, char *nickname, char *caps */
	FC_IM_GOTINFO,
		/* void *connection, void *clientstruct, char *nickname, char *info, int warning, int online, int idle, int flags */
	FC_IM_USER_NICKCHANGED,
		/* void *connection, void *clientstruct, char *oldnick, char *newnick */
	FC_IM_GETMESSAGE,
		/* void *connection, void *clientstruct, char *sender, int automessage_flag, char *message */
	FC_IM_GETACTION,
		/* void *connection, void *clientstruct, char *sender, int automessage_flag, char *message */
	FC_IM_BUDDYONLINE,
		/* void *connection, void *clientstruct, char *nickname */
	FC_IM_BUDDYOFFLINE,
		/* void *connection, void *clientstruct, char *nickname */
	FC_IM_BUDDYAWAY,
		/* void *connection, void *clientstruct, char *nickname */
	FC_IM_BUDDYUNAWAY,
		/* void *connection, void *clientstruct, char *nickname */
	FC_IM_LISTBUDDY,
		/* void *connection, void *clientstruct, char *nickname, char *group, char online, char away, long idletime */
	FC_CHAT_JOINED,
		/* void *connection, void *clientstruct, char *room */
	FC_CHAT_LEFT,
		/* void *connection, void *clientstruct, char *room */
	FC_CHAT_KICKED,
		/* void *connection, void *clientstruct, char *room, char *by, char *reason */
	FC_CHAT_GETMESSAGE,
		/* void *connection, void *clientstruct, char *room, char *from, int automessage_flag, char *message */
	FC_CHAT_GETACTION,
		/* void *connection, void *clientstruct, char *room, char *from, int automessage_flag, char *message */
	FC_CHAT_INVITED,
		/* void *connection, void *clientstruct, char *room, char *from, char *message */
//#ifdef RAWIRCMODES
	FC_CHAT_MODECHANGED,
		/* void *connection, void *clientstruct, char *room, char *mode, char *by */
//#endif
	FC_CHAT_OPPED,
		/* void *connection, void *clientstruct, char *room, char *by */
	FC_CHAT_DEOPPED,
		/* void *connection, void *clientstruct, char *room, char *by */
	FC_CHAT_USER_JOINED,
		/* void *connection, void *clientstruct, char *room, char *who */
	FC_CHAT_USER_LEFT,
		/* void *connection, void *clientstruct, char *room, char *who, char *reason */
	FC_CHAT_GOTTOPIC,
		/* void *connection, void *clientstruct, char *room, char *topic, char *author */
	FC_CHAT_USER_OPPED,
		/* void *connection, void *clientstruct, char *room, char *who, char *by */
	FC_CHAT_USER_DEOPPED,
		/* void *connection, void *clientstruct, char *room, char *who, char *by */
	FC_CHAT_USER_KICKED,
		/* void *connection, void *clientstruct, char *room, char *who, char *by, char *reason */
	FC_CHAT_USER_NICKCHANGED,
		/* void *connection, void *clientstruct, char *room, char *oldnick, char *newnick */
	FC_CHAT_KEYCHANGED,
		/* void *connection, void *clientstruct, char *room, char *what, char *by */
	FC_CHAT_LISTMEMBER,
		/* void *connection, vodi *clientstruct, char *room, char *membername, int opped */
	FC_FILE_OFFER,
		/* void *connection, void *clientstruct, void *filehandle, char *from, char *filename, long size */
	FC_FILE_START,
		/* void *connection, void *clientstruct, void *filehandle, void *clientfilestruct */
	FC_FILE_PROGRESS,
		/* void *connection, void *clientstruct, void *filehandle, void *clientfilestruct, long bytes, long size */
	FC_FILE_FINISH,
		/* void *connection, void *clientstruct, void *filehandle, void *clientfilestruct, long size */
	FC_FILE_ERROR,
		/* void *connection, void *clientstruct, void *filehandle, void *clientfilestruct, int error */
	FC_MAX
		/* tracking enum, don't hook this */
};

typedef enum {
	FE_SUCCESS,
	FE_CONNECT,
	FE_NOMATCH,
	FE_PACKET,
	FE_RECONNECTING,
	FE_BADUSERPASS,
	FE_SEQUENCE,
	FE_FRAMETYPE,
	FE_PACKETSIZE,
	FE_SERVER,
	FE_UNKNOWN,
	FE_BLOCKED,
	FE_WEIRDPACKET,
	FE_CALLBACKNUM,
	FE_BADUSER,
	FE_NOTFOUND,
	FE_DISCONNECT,
	FE_SOCKET,
	FE_RESOLV,
	FE_VERSION,
	FE_USERUNAVAILABLE,
	FE_USERINFOUNAVAILABLE,
	FE_TOOFAST,
	FE_ROOMUNAVAILABLE,
	FE_INCOMINGERROR,
	FE_USERDISCONNECT,
	FE_INVALIDFORMAT,
	FE_IDLEFAST,
	FE_BADROOM,
	FE_BADMESSAGE,
	FE_MESSAGETRUNCATED,
	FE_BADPROTO,
	FE_NOTCONNECTED,
	FE_BADCONNECTION,
	FE_NOPERMS,
	FE_NOCHANGEPASS,
	FE_DUPEROOM,
	FE_IOERROR,
	FE_BADHANDLE,
	FE_TIMEOUT
} fte_t;



/* Firetalk functions */
int	firetalk_find_protocol(const char *strprotocol);
const char *firetalk_strprotocol(const enum firetalk_protocol p);
const char *firetalk_strerror(const fte_t	e);
firetalk_t firetalk_create_handle(const int protocol, void *clientstruct);
void firetalk_destroy_handle(firetalk_t conn);
enum firetalk_protocol firetalk_get_protocol(firetalk_t conn);

fte_t	firetalk_disconnect(firetalk_t conn);
fte_t	firetalk_signon(firetalk_t conn, const char *const server, const short port, const char *const username);
fte_t	firetalk_register_callback(firetalk_t conn, const int type, void (*function)(firetalk_t, void *, ...));
firetalk_t firetalk_find_clientstruct(void *clientstruct);

fte_t	firetalk_im_add_buddy(firetalk_t conn, const char *const name, const char *const group, const char *const friendly);
fte_t	firetalk_im_remove_buddy(firetalk_t conn, const char *const name);
fte_t	firetalk_im_add_deny(firetalk_t conn, const char *const name);
fte_t	firetalk_im_remove_deny(firetalk_t conn, const char *const name);
fte_t	firetalk_im_upload_buddies(firetalk_t conn);
fte_t	firetalk_im_upload_denies(firetalk_t conn);
fte_t	firetalk_im_send_message(firetalk_t conn, const char *const dest, const char *const message, const int auto_flag);
fte_t	firetalk_im_send_action(firetalk_t conn, const char *const dest, const char *const message, const int auto_flag);
fte_t	firetalk_im_list_buddies(firetalk_t conn);
fte_t	firetalk_im_evil(firetalk_t c, const char *const who);
fte_t	firetalk_im_get_info(firetalk_t conn, const char *const nickname);

fte_t	firetalk_chat_join(firetalk_t conn, const char *const room);
fte_t	firetalk_chat_part(firetalk_t conn, const char *const room);
fte_t	firetalk_chat_send_message(firetalk_t conn, const char *const room, const char *const message, const int auto_flag);
fte_t	firetalk_chat_send_action(firetalk_t conn, const char *const room, const char *const message, const int auto_flag);
fte_t	firetalk_chat_invite(firetalk_t conn, const char *const room, const char *const who, const char *const message);
fte_t	firetalk_chat_set_topic(firetalk_t conn, const char *const room, const char *const topic);
fte_t	firetalk_chat_op(firetalk_t conn, const char *const room, const char *const who);
fte_t	firetalk_chat_deop(firetalk_t conn, const char *const room, const char *const who);
fte_t	firetalk_chat_kick(firetalk_t conn, const char *const room, const char *const who, const char *const reason);
fte_t	firetalk_chat_listmembers(firetalk_t conn, const char *const room);

fte_t	firetalk_im_internal_add_deny(firetalk_t conn, const char *const nickname);
fte_t	firetalk_im_internal_remove_buddy(firetalk_t conn, const char *const nickname);
fte_t	firetalk_im_internal_remove_deny(firetalk_t conn, const char *const nickname);
fte_t	firetalk_subcode_send_reply(firetalk_t conn, const char *const to, const char *const command, const char *const args);
fte_t	firetalk_subcode_send_request(firetalk_t conn, const char *const to, const char *const command, const char *const args);

fte_t	firetalk_subcode_register_request_callback(firetalk_t conn, const char *const command, void (*callback)(firetalk_t, void *, const char *const, const char *const, const char *const));
fte_t	firetalk_subcode_register_request_reply(firetalk_t conn, const char *const command, const char *const reply);
fte_t	firetalk_subcode_register_reply_callback(firetalk_t conn, const char *const command, void (*callback)(firetalk_t, void *, const char *const, const char *const, const char *const));

fte_t	firetalk_file_offer(firetalk_t conn, const char *const nickname, const char *const filename, void *clientfilestruct);
fte_t	firetalk_file_accept(firetalk_t conn, void *filehandle, void *clientfilestruct, const char *const localfile);
fte_t	firetalk_file_refuse(firetalk_t conn, void *filehandle);
fte_t	firetalk_file_cancel(firetalk_t conn, void *filehandle);

fte_t	firetalk_compare_nicks(firetalk_t conn, const char *const nick1, const char *const nick2);
fte_t	firetalk_isprint(firetalk_t conn, const int c);
fte_t	firetalk_set_info(firetalk_t conn, const char *const info);
fte_t	firetalk_set_away(firetalk_t c, const char *const message, const int auto_flag);
const char *firetalk_chat_normalize(firetalk_t conn, const char *const room);
fte_t	firetalk_set_nickname(firetalk_t conn, const char *const nickname);
fte_t	firetalk_set_password(firetalk_t conn, const char *const oldpass, const char *const newpass);
fte_t	firetalk_set_privacy(firetalk_t conn, const char *const mode);
fte_t	firetalk_select();
fte_t	firetalk_select_custom(int n, fd_set *fd_read, fd_set *fd_write, fd_set *fd_except, struct timeval *timeout);

#ifndef FIRETALK
extern fte_t	firetalkerror;
/* internal function, exported because dan reed is lazy */
int firetalk_internal_connect_host(const char *const host, const int port);
#endif

#define FF_SUBSTANDARD                  0x0001
#define FF_NORMAL                       0x0002
#define FF_ADMIN                        0x0004

#ifndef MSG_DONTWAIT
# define MSG_DONTWAIT	0
#endif

#ifndef MSG_NOSIGNAL
# define MSG_NOSIGNAL	0
#endif

#endif
