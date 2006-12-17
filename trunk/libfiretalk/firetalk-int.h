/*
firetalk-int.h - FireTalk wrapper declarations
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
#ifndef _FIRETALK_INT_H
#define _FIRETALK_INT_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_INTTYPES_H
# include <inttypes.h>
#endif

#ifdef HAVE_STDINT_H
# include <stdint.h>
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifndef HAVE_INET_ATON
# define inet_aton(x,y)	inet_pton(AF_INET, (x), (y))
#endif

#define DEBUG

#include "firetalk.h"

#ifndef SHUT_RDWR
#define SHUT_RDWR 2
#endif


typedef void (*ptrtofnct)(struct firetalk_connection_t *, struct firetalk_useragent_connection_t *, ...);

typedef struct {
	char	**keys;
	void	**data;
	int	count;
} firetalk_queue_t;

struct s_firetalk_buddy {
	struct s_firetalk_buddy *next;
	char	*nickname,
		*group,
		*friendly;
	long	idletime,
		warnval;
	int	typing;
	char	*capabilities;
	unsigned char online:1,
		away:1,
		uploaded:1;
};

struct s_firetalk_deny {
	struct s_firetalk_deny *next;
	char	*nickname;
	unsigned char uploaded:1;
};

struct s_firetalk_member {
	struct s_firetalk_member *next;
	char *nickname;
	unsigned char admin:1;
};

struct s_firetalk_room {
	struct s_firetalk_room *next;
	struct s_firetalk_member *member_head;
	char	*name;
	unsigned char admin:1,
		sentjoin:1;
};

typedef struct firetalk_transfer_t {
	struct firetalk_transfer_t *next;
	char	*who;
	char	*filename;
	struct in_addr inet_ip;
#ifdef _FC_USE_IPV6
	struct in6_addr inet6_ip;
	int	tryinet6;
#endif
	uint16_t port;
	long	size;
	long	bytes;
	uint32_t acked;
#define FF_STATE_WAITLOCAL	0
#define FF_STATE_WAITREMOTE	1
#define FF_STATE_WAITSYNACK	2
#define FF_STATE_TRANSFERRING	3
	int	state;
#define FF_DIRECTION_SENDING	0
#define FF_DIRECTION_RECEIVING	1
	int	direction;
#define FF_TYPE_DCC		0
#define FF_TYPE_RAW		1
	int	type,
		sockfd,
		filefd;
	struct firetalk_useragent_transfer_t *clientfilestruct;
} firetalk_transfer_t;

struct s_firetalk_subcode_callback {
	struct s_firetalk_subcode_callback *next;
	char	*command, *staticresp;
	ptrtofnct callback;
};

typedef enum {
	FCS_NOTCONNECTED,
	FCS_WAITING_SYNACK,
	FCS_SEND_SIGNON,
	FCS_WAITING_PASSWORD,
	FCS_WAITING_SIGNON,
	FCS_ACTIVE,
} firetalk_sock_state_t;

typedef struct {
	unsigned long canary;
	int	fd;
	firetalk_sock_state_t state;
	struct sockaddr_in remote_addr;
	struct in_addr local_addr;
#ifdef _FC_USE_IPV6
	struct sockaddr_in6 remote_addr6;
	struct in6_addr local_addr6;
#endif
	int	readdata;
	unsigned short bufferpos;
	unsigned char buffer[1024*8];
} firetalk_sock_t;

typedef struct firetalk_connection_t {
	struct firetalk_driver_connection_t *handle;
	struct firetalk_useragent_connection_t *clientstruct;
	int	connected;
	struct sockaddr_in remote_addr;
	struct in_addr local_addr;
#ifdef _FC_USE_IPV6
	struct sockaddr_in6 remote_addr6;
	struct in6_addr local_addr6;
#endif
	unsigned long localip;
	int	protocol;
	char	*username;
	int	fd;
	ptrtofnct callbacks[FC_MAX];
	unsigned char *buffer;
	unsigned short bufferpos;
	struct firetalk_connection_t *next, *prev;
	struct s_firetalk_buddy *buddy_head;
	struct s_firetalk_deny *deny_head;
	struct s_firetalk_room *room_head;
	firetalk_transfer_t *file_head;
	struct s_firetalk_subcode_callback *subcode_request_head,
		*subcode_reply_head,
		*subcode_request_default,
		*subcode_reply_default;
	firetalk_queue_t subcode_requests,
		subcode_replies;
	unsigned char deleted:1;
} firetalk_connection_t;

struct firetalk_driver_connection_t;

typedef struct {
	const char *const strprotocol;
	const char *const default_server;
	const short default_port;
	const unsigned short default_buffersize;
	fte_t	(*periodic)(firetalk_connection_t *const conn);
	fte_t	(*preselect)(struct firetalk_driver_connection_t *c, fd_set *read, fd_set *write, fd_set *except, int *n);
	fte_t	(*postselect)(struct firetalk_driver_connection_t *c, fd_set *read, fd_set *write, fd_set *except);
	fte_t	(*got_data)(struct firetalk_driver_connection_t *c, unsigned char *buffer, unsigned short *bufferpos);
	fte_t	(*got_data_connecting)(struct firetalk_driver_connection_t *c, unsigned char *buffer, unsigned short *bufferpos);
	fte_t	(*comparenicks)(const char *const, const char *const);
	fte_t	(*isprintable)(const int);
	fte_t	(*disconnect)(struct firetalk_driver_connection_t *c);
	fte_t	(*signon)(struct firetalk_driver_connection_t *c, const char *const);
	fte_t	(*get_info)(struct firetalk_driver_connection_t *c, const char *const);
	fte_t	(*set_info)(struct firetalk_driver_connection_t *c, const char *const);
	fte_t	(*set_away)(struct firetalk_driver_connection_t *c, const char *const, const int);
	fte_t	(*set_nickname)(struct firetalk_driver_connection_t *c, const char *const);
	fte_t	(*set_password)(struct firetalk_driver_connection_t *c, const char *const, const char *const);
	fte_t	(*set_privacy)(struct firetalk_driver_connection_t *c, const char *const);
	fte_t	(*im_add_buddy)(struct firetalk_driver_connection_t *c, const char *const, const char *const, const char *const);
	fte_t	(*im_remove_buddy)(struct firetalk_driver_connection_t *c, const char *const, const char *const);
	fte_t	(*im_add_deny)(struct firetalk_driver_connection_t *c, const char *const);
	fte_t	(*im_remove_deny)(struct firetalk_driver_connection_t *c, const char *const);
	fte_t	(*im_send_message)(struct firetalk_driver_connection_t *c, const char *const, const char *const, const int);
	fte_t	(*im_send_action)(struct firetalk_driver_connection_t *c, const char *const, const char *const, const int);
	fte_t	(*im_evil)(struct firetalk_driver_connection_t *c, const char *const);
	fte_t	(*chat_join)(struct firetalk_driver_connection_t *c, const char *const);
	fte_t	(*chat_part)(struct firetalk_driver_connection_t *c, const char *const);
	fte_t	(*chat_invite)(struct firetalk_driver_connection_t *c, const char *const, const char *const, const char *const);
	fte_t	(*chat_set_topic)(struct firetalk_driver_connection_t *c, const char *const, const char *const);
	fte_t	(*chat_op)(struct firetalk_driver_connection_t *c, const char *const, const char *const);
	fte_t	(*chat_deop)(struct firetalk_driver_connection_t *c, const char *const, const char *const);
	fte_t	(*chat_kick)(struct firetalk_driver_connection_t *c, const char *const, const char *const, const char *const);
	fte_t	(*chat_send_message)(struct firetalk_driver_connection_t *c, const char *const, const char *const, const int);
	fte_t	(*chat_send_action)(struct firetalk_driver_connection_t *c, const char *const, const char *const, const int);
	char	*(*subcode_encode)(struct firetalk_driver_connection_t *c, const char *const, const char *const);
	const char *(*room_normalize)(const char *const);
	struct firetalk_driver_connection_t *(*create_handle)(void);
	void	(*destroy_handle)(struct firetalk_driver_connection_t *c);
} firetalk_protocol_t;

fte_t	firetalk_register_protocol(const firetalk_protocol_t *const proto);

void	firetalk_callback_im_getmessage(struct firetalk_driver_connection_t *c, const char *const sender, const int automessage, const char *const message);
void	firetalk_callback_im_getaction(struct firetalk_driver_connection_t *c, const char *const sender, const int automessage, const char *const message);
void	firetalk_callback_im_buddyonline(struct firetalk_driver_connection_t *c, const char *const nickname, const int online);
void	firetalk_callback_im_buddyaway(struct firetalk_driver_connection_t *c, const char *const nickname, const int away);
void	firetalk_callback_buddyadded(struct firetalk_driver_connection_t *c, const char *const name, const char *const group, const char *const friendly);
void	firetalk_callback_buddyremoved(struct firetalk_driver_connection_t *c, const char *const name, const char *const group);
void	firetalk_callback_denyadded(struct firetalk_driver_connection_t *c, const char *const name);
void	firetalk_callback_denyremoved(struct firetalk_driver_connection_t *c, const char *const name);
void	firetalk_callback_typing(struct firetalk_driver_connection_t *c, const char *const name, const int typing);
void	firetalk_callback_capabilities(struct firetalk_driver_connection_t *c, char const *const nickname, const char *const caps);
void	firetalk_callback_warninfo(struct firetalk_driver_connection_t *c, char const *const nickname, const long warnval);
void	firetalk_callback_error(struct firetalk_driver_connection_t *c, const int error, const char *const roomoruser, const char *const description);
void	firetalk_callback_connectfailed(struct firetalk_driver_connection_t *c, const int error, const char *const description);
void	firetalk_callback_connected(struct firetalk_driver_connection_t *c);
void	firetalk_callback_disconnect(struct firetalk_driver_connection_t *c, const int error);
void	firetalk_callback_gotinfo(struct firetalk_driver_connection_t *c, const char *const nickname, const char *const info, const int warning, const long online, const long idle, const int flags);
void	firetalk_callback_idleinfo(struct firetalk_driver_connection_t *c, char const *const nickname, const long idletime);
void	firetalk_callback_doinit(struct firetalk_driver_connection_t *c, char const *const nickname);
void	firetalk_callback_setidle(struct firetalk_driver_connection_t *c, long *const idle);
void	firetalk_callback_eviled(struct firetalk_driver_connection_t *c, const int newevil, const char *const eviler);
void	firetalk_callback_newnick(struct firetalk_driver_connection_t *c, const char *const nickname);
void	firetalk_callback_passchanged(struct firetalk_driver_connection_t *c);
void	firetalk_callback_user_nickchanged(struct firetalk_driver_connection_t *c, const char *const oldnick, const char *const newnick);
void	firetalk_callback_chat_joined(struct firetalk_driver_connection_t *c, const char *const room);
void	firetalk_callback_chat_left(struct firetalk_driver_connection_t *c, const char *const room);
void	firetalk_callback_chat_kicked(struct firetalk_driver_connection_t *c, const char *const room, const char *const by, const char *const reason);
void	firetalk_callback_chat_getmessage(struct firetalk_driver_connection_t *c, const char *const room, const char *const from, const int automessage, const char *const message);
void	firetalk_callback_chat_getaction(struct firetalk_driver_connection_t *c, const char *const room, const char *const from, const int automessage, const char *const message);
void	firetalk_callback_chat_invited(struct firetalk_driver_connection_t *c, const char *const room, const char *const from, const char *const message);
void	firetalk_callback_chat_user_joined(struct firetalk_driver_connection_t *c, const char *const room, const char *const who, const char *const extra);
void	firetalk_callback_chat_user_left(struct firetalk_driver_connection_t *c, const char *const room, const char *const who, const char *const reason);
void	firetalk_callback_chat_user_quit(struct firetalk_driver_connection_t *c, const char *const who, const char *const reason);
void	firetalk_callback_chat_gottopic(struct firetalk_driver_connection_t *c, const char *const room, const char *const topic, const char *const author);
#ifdef RAWIRCMODES
void	firetalk_callback_chat_modechanged(struct firetalk_driver_connection_t *c, const char *const room, const char *const mode, const char *const by);
#endif
void	firetalk_callback_chat_user_opped(struct firetalk_driver_connection_t *c, const char *const room, const char *const who, const char *const by);
void	firetalk_callback_chat_user_deopped(struct firetalk_driver_connection_t *c, const char *const room, const char *const who, const char *const by);
void	firetalk_callback_chat_keychanged(struct firetalk_driver_connection_t *c, const char *const room, const char *const what, const char *const by);
void	firetalk_callback_chat_opped(struct firetalk_driver_connection_t *c, const char *const room, const char *const by);
void	firetalk_callback_chat_deopped(struct firetalk_driver_connection_t *c, const char *const room, const char *const by);
void	firetalk_callback_chat_user_kicked(struct firetalk_driver_connection_t *c, const char *const room, const char *const who, const char *const by, const char *const reason);
const char *firetalk_subcode_get_request_reply(struct firetalk_driver_connection_t *c, const char *const command);
void	firetalk_callback_subcode_request(struct firetalk_driver_connection_t *c, const char *const from, const char *const command, char *args);
void	firetalk_callback_subcode_reply(struct firetalk_driver_connection_t *c, const char *const from, const char *const command, const char *const args);
void	firetalk_callback_file_offer(struct firetalk_driver_connection_t *c, const char *const from, const char *const filename, const long size, const char *const ipstring, const char *const ip6string, const uint16_t port, const int type);
void	firetalk_callback_needpass(struct firetalk_driver_connection_t *c, char *pass, const int size);

void	firetalk_enqueue(firetalk_queue_t *queue, const char *const key, void *data);
const void *firetalk_peek(firetalk_queue_t *queue, const char *const key);
void	*firetalk_dequeue(firetalk_queue_t *queue, const char *const key);
void	firetalk_queue_append(char *buf, int buflen, firetalk_queue_t *queue, const char *const key);

firetalk_connection_t *firetalk_find_handle(struct firetalk_driver_connection_t *c);

fte_t	firetalk_chat_internal_add_room(firetalk_connection_t *conn, const char *const name);
fte_t	firetalk_chat_internal_add_member(firetalk_connection_t *conn, const char *const room, const char *const nickname);
fte_t	firetalk_chat_internal_remove_room(firetalk_connection_t *conn, const char *const name);
fte_t	firetalk_chat_internal_remove_member(firetalk_connection_t *conn, const char *const room, const char *const nickname);

struct s_firetalk_room *firetalk_find_room(firetalk_connection_t *c, const char *const room);
fte_t	firetalk_user_visible(firetalk_connection_t *conn, const char *const nickname);
fte_t	firetalk_user_visible_but(firetalk_connection_t *conn, const char *const room, const char *const nickname);

void	firetalk_handle_send(firetalk_connection_t *c, firetalk_transfer_t *filestruct);
void	firetalk_handle_receive(firetalk_connection_t *c, firetalk_transfer_t *filestruct);

void	firetalk_internal_send_data(firetalk_connection_t *c, const char *const data, const int length);

int	firetalk_internal_connect_host(const char *const host, const int port);
int	firetalk_internal_connect(struct sockaddr_in *inet4_ip
#ifdef _FC_USE_IPV6
		, struct sockaddr_in6 *inet6_ip
#endif
		);
fte_t	firetalk_internal_resolve4(const char *const host, struct in_addr *inet4_ip);
struct sockaddr_in *firetalk_internal_remotehost4(struct firetalk_driver_connection_t *c);
#ifdef _FC_USE_IPV6
fte_t	firetalk_internal_resolve6(const char *const host, struct in6_addr *inet6_ip);
struct sockaddr_in6 *firetalk_internal_remotehost6(struct firetalk_driver_connection_t *c);
#endif
firetalk_sock_state_t firetalk_internal_get_connectstate(struct firetalk_driver_connection_t *c);
void	firetalk_internal_set_connectstate(struct firetalk_driver_connection_t *c, firetalk_sock_state_t fcs);

fte_t	firetalk_sock_connect_host(firetalk_sock_t *sock, const char *const host, const uint16_t port);
fte_t	firetalk_sock_connect(firetalk_sock_t *sock);
fte_t	firetalk_sock_resolve4(const char *const host, struct in_addr *inet4_ip);
struct sockaddr_in *firetalk_sock_remotehost4(firetalk_sock_t *sock);
#ifdef _FC_USE_IPV6
fte_t	firetalk_sock_resolve6(const char *const host, struct in6_addr *inet6_ip);
struct sockaddr_in6 *firetalk_sock_remotehost6(firetalk_sock_t *sock);
#endif

void	firetalk_sock_init(firetalk_sock_t *sock);
void	firetalk_sock_close(firetalk_sock_t *sock);
fte_t	firetalk_sock_send(firetalk_sock_t *sock, const char *const buffer, const int bufferlen);
void	firetalk_sock_preselect(firetalk_sock_t *sock, fd_set *my_read, fd_set *my_write, fd_set *my_except, int *n);
fte_t	firetalk_sock_postselect(firetalk_sock_t *sock, fd_set *my_read, fd_set *my_write, fd_set *my_except);

char	*firetalk_htmlclean(const char *str);
const char *firetalk_nhtmlentities(const char *str, int len);
const char *firetalk_htmlentities(const char *str);
const char *firetalk_debase64(const char *const str);

#endif
