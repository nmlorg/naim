/*  _ __   __ _ ___ __  __
** | '_ \ / _` |_ _|  \/  | naim
** | | | | (_| || || |\/| | Copyright 1998-2006 Daniel Reed <n@ml.org>
** |_| |_|\__,_|___|_|  |_| ncurses-based chat client
*/
#include <naim/naim.h>
#include <naim/modutil.h>
#include "naim-int.h"

extern conn_t	*curconn;
extern win_t	win_input;
extern char	**names;
extern int	namec;
extern namescomplete_t namescomplete;

#define nFIRE_HANDLER(func) \
static void _firebind_ ## func(struct firetalk_connection_t *sess, conn_t *conn, ...)

#define nFIRE_CTCPHAND(func) \
static void _firebind_ ## func(struct firetalk_connection_t *sess, conn_t *conn, const char *from, const char *command, const char *args)

HOOK_DECLARE(proto_newnick);
nFIRE_HANDLER(newnick) {
	va_list	msg;
	const char *newnick;

	va_start(msg, conn);
	newnick = va_arg(msg, const char *);
	va_end(msg);

	HOOK_CALL(proto_newnick, conn, newnick);
}

HOOK_DECLARE(proto_nickchanged);
nFIRE_HANDLER(nickchanged) {
	va_list	msg;
	const char *oldnick, *newnick;

	va_start(msg, conn);
	oldnick = va_arg(msg, const char *);
	newnick = va_arg(msg, const char *);
	va_end(msg);

	HOOK_CALL(proto_nickchanged, conn, oldnick, newnick);
}

HOOK_DECLARE(proto_doinit);
nFIRE_HANDLER(doinit) {
	va_list	msg;
	const char *screenname;

	va_start(msg, conn);
	screenname = va_arg(msg, const char *);
	va_end(msg);

	HOOK_CALL(proto_doinit, conn, screenname);
}

nFIRE_HANDLER(setidle) {
	va_list	msg;
	long	*idle, idletime = script_getvar_int("idletime");

	va_start(msg, conn);
	idle = va_arg(msg, long *);
	va_end(msg);

	if ((*idle)/60 != idletime)
		*idle = 60*idletime;
}

HOOK_DECLARE(proto_warned);
nFIRE_HANDLER(warned) {
	va_list	msg;
	const char *who;
	int	newlev;

	va_start(msg, conn);
	newlev = va_arg(msg, int);
	who = va_arg(msg, const char *);
	va_end(msg);

	HOOK_CALL(proto_warned, conn, newlev, who);
}

HOOK_DECLARE(proto_buddy_idle);
nFIRE_HANDLER(buddy_idle) {
	va_list	msg;
	const char *who;
	long	idletime;

	va_start(msg, conn);
	who = va_arg(msg, const char *);
	idletime = va_arg(msg, long);
	va_end(msg);

	HOOK_CALL(proto_buddy_idle, conn, who, idletime);
}

HOOK_DECLARE(proto_buddy_eviled);
nFIRE_HANDLER(buddy_eviled) {
	va_list	msg;
	const char *who;
	long	warnval;

	va_start(msg, conn);
	who = va_arg(msg, const char *);
	warnval = va_arg(msg, long);
	va_end(msg);

	HOOK_CALL(proto_buddy_eviled, conn, who, warnval);
}

HOOK_DECLARE(proto_buddy_caps);
nFIRE_HANDLER(buddy_caps) {
	va_list	msg;
	const char *who, *caps;

	va_start(msg, conn);
	who = va_arg(msg, const char *);
	caps = va_arg(msg, const char *);
	va_end(msg);

	HOOK_CALL(proto_buddy_caps, conn, who, caps);
}

HOOK_DECLARE(proto_buddy_typing);
nFIRE_HANDLER(buddy_typing) {
	va_list	msg;
	const char *who;
	int	typing;

	va_start(msg, conn);
	who = va_arg(msg, const char *);
	typing = va_arg(msg, int);
	va_end(msg);

	HOOK_CALL(proto_buddy_typing, conn, who, typing);
}

HOOK_DECLARE(proto_buddy_away);
nFIRE_HANDLER(buddy_away) {
	va_list	msg;
	const char *who;

	va_start(msg, conn);
	who = va_arg(msg, const char *);
	va_end(msg);

	HOOK_CALL(proto_buddy_away, conn, who);
}

HOOK_DECLARE(proto_buddy_unaway);
nFIRE_HANDLER(buddy_unaway) {
	va_list	msg;
	const char *who;

	va_start(msg, conn);
	who = va_arg(msg, const char *);
	va_end(msg);

	HOOK_CALL(proto_buddy_unaway, conn, who);
}

HOOK_DECLARE(proto_buddyadded);
nFIRE_HANDLER(buddyadded) {
	va_list	msg;
	const char *screenname, *group, *friendly;

	va_start(msg, conn);
	screenname = va_arg(msg, const char *);
	group = va_arg(msg, const char *);
	friendly = va_arg(msg, const char *);
	va_end(msg);

	HOOK_CALL(proto_buddyadded, conn, screenname, group, friendly);
}

HOOK_DECLARE(proto_buddyremoved);
nFIRE_HANDLER(buddyremoved) {
	va_list	msg;
	const char *screenname;

	va_start(msg, conn);
	screenname = va_arg(msg, const char *);
	va_end(msg);

	HOOK_CALL(proto_buddyremoved, conn, screenname);
}

HOOK_DECLARE(proto_denyadded);
nFIRE_HANDLER(denyadded) {
	va_list	msg;
	const char *screenname;

	va_start(msg, conn);
	screenname = va_arg(msg, const char *);
	va_end(msg);

	HOOK_CALL(proto_denyadded, conn, screenname);
}

HOOK_DECLARE(proto_denyremoved);
nFIRE_HANDLER(denyremoved) {
	va_list	msg;
	const char *screenname;

	va_start(msg, conn);
	screenname = va_arg(msg, const char *);
	va_end(msg);

	HOOK_CALL(proto_denyremoved, conn, screenname);
}

HOOK_DECLARE(proto_buddy_coming);
nFIRE_HANDLER(buddy_coming) {
	va_list	msg;
	const char *who;

	va_start(msg, conn);
	who = va_arg(msg, const char *);
	va_end(msg);

	HOOK_CALL(proto_buddy_coming, conn, who);
}

HOOK_DECLARE(proto_buddy_going);
nFIRE_HANDLER(buddy_going) {
	va_list	msg;
	const char *who;

	va_start(msg, conn);
	who = va_arg(msg, const char *);
	va_end(msg);

	HOOK_CALL(proto_buddy_going, conn, who);
}

HOOK_DECLARE(proto_recvfrom);
static void naim_recvfrom(conn_t *const conn,
		const char *const _name, 
		const char *const _dest,
		const unsigned char *_message, int len,
		int flags) {
 	char	*name = NULL, *dest = NULL;
	unsigned char *message = malloc(len+1);

	if (_name != NULL)
		name = strdup(_name);
	if (_dest != NULL)
		dest = strdup(_dest);

	memmove(message, _message, len);
	message[len] = 0;
	HOOK_CALL(proto_recvfrom, conn, &name, &dest, &message, &len, &flags);
	free(name);
	free(dest);
	free(message);
}

static void do_replace(unsigned char *dest, const unsigned char *new, int wordlen, int len) {
	int	newlen = strlen(new);

	if (newlen > wordlen)
		memmove(dest+newlen, dest+wordlen, len-newlen);
	else if (newlen < wordlen)
		memmove(dest+newlen, dest+wordlen, len-wordlen);

	memmove(dest, new, newlen);
}

static void str_replace(const unsigned char *orig, const unsigned char *new, unsigned char *str, int strsize) {
	int	i, l = strlen(orig);

	assert(*str != 0);

	for (i = 0; (str[i] != 0) && (i+l < strsize); i++) {
		if (i > 0)
			switch (str[i-1]) {
				case ' ':
				case '>':
				case '(':
					break;
				default:
					continue;
			}
		switch (str[i+l]) {
			case ' ':
			case ',':
			case '.':
			case '!':
			case '?':
			case '<':
			case ')':
			case 0:
				break;
			default:
				continue;
		}
		if (strncmp(str+i, orig, l) == 0)
			do_replace(str+i, new, l, strsize-i);
	}
}

html_clean_t *html_cleanar = NULL;
int	html_cleanc = 0;

static const unsigned char *html_clean(const unsigned char *str) {
	static unsigned char buf[1024*4];
	int	i;

	assert(str != NULL);
	if (*str == 0)
		return(str);
	strncpy(buf, str, sizeof(buf)-1);
	buf[sizeof(buf)-1] = 0;
	for (i = 0; i < html_cleanc; i++)
		str_replace(html_cleanar[i].from, html_cleanar[i].replace, buf, sizeof(buf));
	return(buf);
}

nFIRE_HANDLER(im_handle) {
	va_list	msg;
	const char *name;
	int	isautoreply;
	const unsigned char *message;

	va_start(msg, conn);
	name = va_arg(msg, const char *);
	isautoreply = va_arg(msg, int);
	message = html_clean(va_arg(msg, const unsigned char *));
	va_end(msg);

	assert(message != NULL);

	naim_recvfrom(conn, name, NULL, message, strlen(message),
		isautoreply?RF_AUTOMATIC:RF_NONE);
}

nFIRE_HANDLER(act_handle) {
	va_list	msg;
	const char *who;
	int	isautoreply;
	const unsigned char *message;

	va_start(msg, conn);
	who = va_arg(msg, const char *);
	isautoreply = va_arg(msg, int);
	message = va_arg(msg, const unsigned char *);
	va_end(msg);

	if (message == NULL)
		message = "";

	naim_recvfrom(conn, who, NULL, message, strlen(message),
		isautoreply?RF_AUTOMATIC:RF_NONE | RF_ACTION);
}

nFIRE_HANDLER(chat_getmessage) {
	va_list	msg;
	const char *room, *who;
	int	isautoreply;
	const unsigned char *message;

	va_start(msg, conn);
	room = va_arg(msg, const char *);
	who = va_arg(msg, const char *);
	isautoreply = va_arg(msg, int);
	message = html_clean(va_arg(msg, const unsigned char *));
	va_end(msg);

	assert(who != NULL);
	assert(message != NULL);

	if ((conn->sn != NULL) && (firetalk_compare_nicks(conn->conn, who, conn->sn) == FE_SUCCESS))
		return;

	naim_recvfrom(conn, who, room, message, strlen(message),
		isautoreply?RF_AUTOMATIC:RF_NONE);
}

nFIRE_HANDLER(chat_act_handle) {
	va_list	msg;
	const char *room, *who;
	int	isautoreply;
	const unsigned char *message;

	va_start(msg, conn);
	room = va_arg(msg, const char *);
	who = va_arg(msg, const char *);
	isautoreply = va_arg(msg, int);
	message = va_arg(msg, const unsigned char *);
	va_end(msg);

	if (firetalk_compare_nicks(conn->conn, who, conn->sn) == FE_SUCCESS)
		return;

	if (message == NULL)
		message = "";

	naim_recvfrom(conn, who, room, message, strlen(message),
		isautoreply?RF_AUTOMATIC:RF_NONE | RF_ACTION);
}

void	naim_awaylog(conn_t *conn, const char *src, const char *msg) {
	naim_recvfrom(conn, src, ":AWAYLOG", msg, strlen(msg), RF_NOLOG);
}

HOOK_DECLARE(proto_connected);
nFIRE_HANDLER(connected) {
	HOOK_CALL(proto_connected, conn);
}

HOOK_DECLARE(proto_connectfailed);
nFIRE_HANDLER(connectfailed) {
	va_list	msg;
	int	err;
	const char *reason;

	va_start(msg, conn);
	err = va_arg(msg, int);
	reason = va_arg(msg, const char *);
	va_end(msg);

	HOOK_CALL(proto_connectfailed, conn, err, reason);
}

HOOK_DECLARE(proto_error_msg);
nFIRE_HANDLER(error_msg) {
	va_list	msg;
	int	error;
	const char *target, *desc;

	va_start(msg, conn);
	error = va_arg(msg, int);
	target = va_arg(msg, const char *);
	desc = va_arg(msg, const char *);
	va_end(msg);

	HOOK_CALL(proto_error_msg, conn, error, target, desc);
}

HOOK_DECLARE(proto_error_disconnect);
nFIRE_HANDLER(error_disconnect) {
	va_list	msg;
	int	error;

	va_start(msg, conn);
	error = va_arg(msg, int);
	va_end(msg);

	HOOK_CALL(proto_error_disconnect, conn, error);
}

nFIRE_HANDLER(needpass) {
	va_list	msg;
	char	*pass;
	int	len;
	const char *mypass;

	va_start(msg, conn);
	pass = va_arg(msg, char *);
	len = va_arg(msg, int);
	va_end(msg);

	assert(len > 1);

	if ((mypass = getvar(conn, "password")) == NULL) {
		if (conn != curconn)
			curconn = conn;
		echof(conn, NULL, "Password required to connect to %s.\n",
			conn->winname);
		echof(conn, NULL, "Please type your password and press Enter.\n");
		nw_getpass(&win_input, pass, len);
		nw_erase(&win_input);
		statrefresh();
	} else {
		strncpy(pass, mypass, len-1);
		pass[len-1] = 0;
	}
}

HOOK_DECLARE(proto_userinfo);
nFIRE_HANDLER(userinfo_handler) {
	va_list	msg;
	const char *SN;
	const unsigned char *info;
	long	warning, online, idle, class;

	va_start(msg, conn);
	SN = va_arg(msg, const char *);
	info = va_arg(msg, const unsigned char *);
	warning = va_arg(msg, long);
	online = va_arg(msg, long);
	idle = va_arg(msg, long);
	class = va_arg(msg, long);
	va_end(msg);

	HOOK_CALL(proto_userinfo, conn, SN, info, warning, online, idle, class);
}

HOOK_DECLARE(proto_chat_joined);
nFIRE_HANDLER(chat_joined) {
	va_list	msg;
	const char *room;

	va_start(msg, conn);
	room = va_arg(msg, const char *);
	va_end(msg);

	HOOK_CALL(proto_chat_joined, conn, room);
}

HOOK_DECLARE(proto_chat_left);
nFIRE_HANDLER(chat_left) {
	va_list	msg;
	const char *room;

	va_start(msg, conn);
	room = va_arg(msg, const char *);
	va_end(msg);

	HOOK_CALL(proto_chat_left, conn, room);
}

HOOK_DECLARE(proto_chat_kicked);
nFIRE_HANDLER(chat_kicked) {
	va_list	msg;
	const char *room, *by, *reason;

	va_start(msg, conn);
	room = va_arg(msg, const char *);
	by = va_arg(msg, const char *);
	reason = va_arg(msg, const char *);
	va_end(msg);

	HOOK_CALL(proto_chat_kicked, conn, room, by, reason);
}

HOOK_DECLARE(proto_chat_invited);
nFIRE_HANDLER(chat_invited) {
	va_list	msg;
	const char *room, *who, *message;

	va_start(msg, conn);
	room = va_arg(msg, const char *);
	who = va_arg(msg, const char *);
	message = va_arg(msg, const char *);
	va_end(msg);

	HOOK_CALL(proto_chat_invited, conn, room, who, message);
}

HOOK_DECLARE(proto_chat_user_joined);
nFIRE_HANDLER(chat_JOIN) {
	va_list	msg;
	const char *room, *who, *extra;

	va_start(msg, conn);
	room = va_arg(msg, const char *);
	who = va_arg(msg, const char *);
	extra = va_arg(msg, const char *);
	va_end(msg);

	HOOK_CALL(proto_chat_user_joined, conn, room, who, extra);
}

HOOK_DECLARE(proto_chat_user_left);
nFIRE_HANDLER(chat_PART) {
	va_list	msg;
	const char *room, *who, *reason;

	va_start(msg, conn);
	room = va_arg(msg, const char *);
	who = va_arg(msg, const char *);
	reason = va_arg(msg, const char *);
	va_end(msg);

	HOOK_CALL(proto_chat_user_left, conn, room, who, reason);
}

HOOK_DECLARE(proto_chat_user_kicked);
nFIRE_HANDLER(chat_KICK) {
	va_list	msg;
	const char *room, *who, *by, *reason;

	va_start(msg, conn);
	room = va_arg(msg, const char *);
	who = va_arg(msg, const char *);
	by = va_arg(msg, const char *);
	reason = va_arg(msg, const char *);
	va_end(msg);

	HOOK_CALL(proto_chat_user_kicked, conn, room, who, by, reason);
}

HOOK_DECLARE(proto_chat_keychanged);
nFIRE_HANDLER(chat_KEYCHANGED) {
	va_list	msg;
	const char *room, *what, *by;

	va_start(msg, conn);
	room = va_arg(msg, const char *);
	what = va_arg(msg, const char *);
	by = va_arg(msg, const char *);
	va_end(msg);

	HOOK_CALL(proto_chat_keychanged, conn, room, what, by);
}

#ifdef RAWIRCMODES
HOOK_DECLARE(proto_chat_modechanged);
nFIRE_HANDLER(chat_MODECHANGED) {
	va_list	msg;
	const char *room, *mode, *by;

	va_start(msg, conn);
	room = va_arg(msg, const char *);
	mode = va_arg(msg, const char *);
	by = va_arg(msg, const char *);
	va_end(msg);

	HOOK_CALL(proto_chat_modechanged, conn, room, made, by);
}
#endif

HOOK_DECLARE(proto_chat_oped);
nFIRE_HANDLER(chat_oped) {
	va_list	msg;
	const char *room, *by;

	va_start(msg, conn);
	room = va_arg(msg, const char *);
	by = va_arg(msg, const char *);
	va_end(msg);

	HOOK_CALL(proto_chat_oped, conn, room, by);
}

HOOK_DECLARE(proto_chat_user_oped);
nFIRE_HANDLER(chat_OP) {
	va_list	msg;
	const char *room, *who, *by;

	va_start(msg, conn);
	room = va_arg(msg, const char *);
	who = va_arg(msg, const char *);
	by = va_arg(msg, const char *);
	va_end(msg);

	HOOK_CALL(proto_chat_user_oped, conn, room, who, by);
}

HOOK_DECLARE(proto_chat_deoped);
nFIRE_HANDLER(chat_deoped) {
	va_list	msg;
	const char *room, *by;

	va_start(msg, conn);
	room = va_arg(msg, const char *);
	by = va_arg(msg, const char *);
	va_end(msg);

	HOOK_CALL(proto_chat_deoped, conn, room, by);
}

HOOK_DECLARE(proto_chat_user_deoped);
nFIRE_HANDLER(chat_DEOP) {
	va_list	msg;
	const char *room, *who, *by;

	va_start(msg, conn);
	room = va_arg(msg, const char *);
	who = va_arg(msg, const char *);
	by = va_arg(msg, const char *);
	va_end(msg);

	HOOK_CALL(proto_chat_user_deoped, conn, room, who, by);
}

HOOK_DECLARE(proto_chat_topicchanged);
nFIRE_HANDLER(chat_TOPIC) {
	va_list	msg;
	const char *room, *topic, *by;

	va_start(msg, conn);
	room = va_arg(msg, const char *);
	topic = va_arg(msg, const char *);
	by = va_arg(msg, const char *);
	va_end(msg);

	HOOK_CALL(proto_chat_topicchanged, conn, room, topic, by);
}

HOOK_DECLARE(proto_chat_user_nickchanged);
nFIRE_HANDLER(chat_NICK) {
	va_list	msg;
	const char *room, *oldnick, *newnick;

	va_start(msg, conn);
	room = va_arg(msg, const char *);
	oldnick = va_arg(msg, const char *);
	newnick = va_arg(msg, const char *);
	va_end(msg);

	HOOK_CALL(proto_chat_user_nickchanged, conn, room, oldnick, newnick);
}

nFIRE_HANDLER(chat_NAMES) {
	va_list	msg;
	const char *room, *nick;
	int	oped;
//	int	i, j;

	va_start(msg, conn);
	room = va_arg(msg, const char *);
	nick = va_arg(msg, const char *);
	oped = va_arg(msg, int);
	va_end(msg);

	if (namescomplete.buf != NULL) {
		assert(namescomplete.len > 0);
		if (namescomplete.foundmatch) {
			if (!namescomplete.foundmult && (strncasecmp(nick, namescomplete.buf, namescomplete.len) == 0))
				namescomplete.foundmult = 1;
			return;
		}
		if (strlen(namescomplete.buf) > namescomplete.len) {
			int	len = strlen(namescomplete.buf);

			assert(len > 0);
			if (namescomplete.buf[len-1] == ' ');
				len--;
			assert(len > 0);
			if (namescomplete.buf[len-1] == ',');
				len--;
			assert(len > 0);
			if (strncmp(namescomplete.buf, nick, len) == 0) {
				namescomplete.foundmult = namescomplete.foundfirst = 1;
				return;
			} else if (!namescomplete.foundfirst) {
				if (!namescomplete.foundmult && (strncasecmp(nick, namescomplete.buf, namescomplete.len) == 0))
					namescomplete.foundmult = 1;
				return;
			}
		}
		if (strncasecmp(nick, namescomplete.buf, namescomplete.len) == 0) {
			free(namescomplete.buf);
			namescomplete.buf = strdup(nick);
			namescomplete.foundmatch = 1;
		}
		return;
	}

	namec++;
	names = realloc(names, namec*sizeof(*names));
	names[namec-1] = malloc(strlen(nick) + oped + 1);
	sprintf(names[namec-1], "%s%s", oped?"@":"", nick);
//	for (i = 0, j = strlen(namesbuf); nick[i] != 0; i++, j++)
//		if (isspace(nick[i]))
//			namesbuf[j] = '_';
//		else
//			namesbuf[j] = nick[i];
//	namesbuf[j] = ' ';
//	namesbuf[j+1] = 0;
}

static int qsort_strcasecmp(const void *p1, const void *p2) {
	register char **b1 = (char **)p1, **b2 = (char **)p2;

	return(strcasecmp(*b1, *b2));
}

void	naim_chat_listmembers(conn_t *conn, const char *const chat) {
	firetalk_chat_listmembers(conn->conn, chat);
	if (names != NULL)
		qsort(names, namec, sizeof(*names), qsort_strcasecmp);
}

HOOK_DECLARE(proto_file_offer);
nFIRE_HANDLER(file_offer) {
	va_list	msg;
	struct firetalk_transfer_t *handle;
	const char *from, *filename;
	long	size;

	va_start(msg, conn);
	handle = va_arg(msg, struct firetalk_transfer_t *);
	from = va_arg(msg, const char *);
	filename = va_arg(msg, const char *);
	size = va_arg(msg, long);
	va_end(msg);

	HOOK_CALL(proto_file_offer, conn, handle, from, filename, size);
}

HOOK_DECLARE(proto_file_start);
nFIRE_HANDLER(file_start) {
	va_list	msg;
	struct firetalk_transfer_t *handle;
	transfer_t *transfer;

	va_start(msg, conn);
	handle = va_arg(msg, struct firetalk_transfer_t *);
	transfer = va_arg(msg, transfer_t *);
	va_end(msg);

	HOOK_CALL(proto_file_start, conn, handle, transfer);
}

HOOK_DECLARE(proto_file_progress);
nFIRE_HANDLER(file_progress) {
	va_list	msg;
	struct firetalk_transfer_t *handle;
	transfer_t *transfer;
	long	bytes,
		size;

	va_start(msg, conn);
	handle = va_arg(msg, struct firetalk_transfer_t *);
	transfer = va_arg(msg, transfer_t *);
	bytes = va_arg(msg, long);
	size = va_arg(msg, long);
	va_end(msg);

	HOOK_CALL(proto_file_progress, conn, handle, transfer, bytes, size);
}

HOOK_DECLARE(proto_file_finish);
nFIRE_HANDLER(file_finish) {
	va_list	msg;
	struct firetalk_transfer_t *handle;
	transfer_t *transfer;
	long	size;

	va_start(msg, conn);
	handle = va_arg(msg, struct firetalk_transfer_t *);
	transfer = va_arg(msg, transfer_t *);
	size = va_arg(msg, long);
	va_end(msg);

	HOOK_CALL(proto_file_finish, conn, handle, transfer, size);
}

HOOK_DECLARE(proto_file_error);
nFIRE_HANDLER(file_error) {
	va_list	msg;
	struct firetalk_transfer_t *handle;
	transfer_t *transfer;
	int	error;

	va_start(msg, conn);
	handle = va_arg(msg, struct firetalk_transfer_t *);
	transfer = va_arg(msg, transfer_t *);
	error = va_arg(msg, int);
	va_end(msg);

	HOOK_CALL(proto_file_error, conn, handle, transfer, error);
}

conn_t	*naim_newconn(int proto) {
	conn_t	*conn = calloc(1, sizeof(conn_t));

	assert(conn != NULL);

	conn->proto = proto;
	naim_lastupdate(conn);

	{
		conn->conn = firetalk_create_conn(proto, conn);
		firetalk_register_callback(conn->conn, FC_DOINIT,			_firebind_doinit);
		firetalk_register_callback(conn->conn, FC_CONNECTED,			_firebind_connected);
		firetalk_register_callback(conn->conn, FC_CONNECTFAILED,		_firebind_connectfailed);
		firetalk_register_callback(conn->conn, FC_ERROR,			_firebind_error_msg);
		firetalk_register_callback(conn->conn, FC_DISCONNECT,			_firebind_error_disconnect);
		firetalk_register_callback(conn->conn, FC_SETIDLE,			_firebind_setidle);

		firetalk_register_callback(conn->conn, FC_EVILED,			_firebind_warned);
		firetalk_register_callback(conn->conn, FC_NEWNICK,			_firebind_newnick);
		/* FC_PASSCHANGED */

		firetalk_register_callback(conn->conn, FC_IM_GOTINFO,			_firebind_userinfo_handler);
		firetalk_register_callback(conn->conn, FC_IM_USER_NICKCHANGED,		_firebind_nickchanged);
		firetalk_register_callback(conn->conn, FC_IM_GETMESSAGE,		_firebind_im_handle);
		firetalk_register_callback(conn->conn, FC_IM_GETACTION,			_firebind_act_handle);
		firetalk_register_callback(conn->conn, FC_IM_BUDDYADDED,		_firebind_buddyadded);
		firetalk_register_callback(conn->conn, FC_IM_BUDDYREMOVED,		_firebind_buddyremoved);
		firetalk_register_callback(conn->conn, FC_IM_DENYADDED,			_firebind_denyadded);
		firetalk_register_callback(conn->conn, FC_IM_DENYREMOVED,		_firebind_denyremoved);
		firetalk_register_callback(conn->conn, FC_IM_BUDDYONLINE,		_firebind_buddy_coming);
		firetalk_register_callback(conn->conn, FC_IM_BUDDYOFFLINE,		_firebind_buddy_going);
		firetalk_register_callback(conn->conn, FC_IM_BUDDYAWAY,			_firebind_buddy_away);
		firetalk_register_callback(conn->conn, FC_IM_BUDDYUNAWAY,		_firebind_buddy_unaway);
		firetalk_register_callback(conn->conn, FC_IM_IDLEINFO,			_firebind_buddy_idle);
		firetalk_register_callback(conn->conn, FC_IM_TYPINGINFO,		_firebind_buddy_typing);
		firetalk_register_callback(conn->conn, FC_IM_EVILINFO,			_firebind_buddy_eviled);
		firetalk_register_callback(conn->conn, FC_IM_CAPABILITIES,		_firebind_buddy_caps);

		firetalk_register_callback(conn->conn, FC_CHAT_JOINED,			_firebind_chat_joined);
		firetalk_register_callback(conn->conn, FC_CHAT_LEFT,			_firebind_chat_left);
		firetalk_register_callback(conn->conn, FC_CHAT_KICKED,			_firebind_chat_kicked);
		firetalk_register_callback(conn->conn, FC_CHAT_KEYCHANGED,		_firebind_chat_KEYCHANGED);
		firetalk_register_callback(conn->conn, FC_CHAT_GETMESSAGE,		_firebind_chat_getmessage);
		firetalk_register_callback(conn->conn, FC_CHAT_GETACTION,		_firebind_chat_act_handle);
		firetalk_register_callback(conn->conn, FC_CHAT_INVITED,			_firebind_chat_invited);
#ifdef RAWIRCMODES
		firetalk_register_callback(conn->conn, FC_CHAT_MODECHANGED,		_firebind_chat_MODECHANGED);
#endif
		firetalk_register_callback(conn->conn, FC_CHAT_OPPED,			_firebind_chat_oped);
		firetalk_register_callback(conn->conn, FC_CHAT_DEOPPED,			_firebind_chat_deoped);
		firetalk_register_callback(conn->conn, FC_CHAT_USER_JOINED,		_firebind_chat_JOIN);
		firetalk_register_callback(conn->conn, FC_CHAT_USER_LEFT,		_firebind_chat_PART);
		firetalk_register_callback(conn->conn, FC_CHAT_GOTTOPIC,		_firebind_chat_TOPIC);
		firetalk_register_callback(conn->conn, FC_CHAT_USER_OPPED,		_firebind_chat_OP);
		firetalk_register_callback(conn->conn, FC_CHAT_USER_DEOPPED,		_firebind_chat_DEOP);
		firetalk_register_callback(conn->conn, FC_CHAT_USER_KICKED,		_firebind_chat_KICK);
		firetalk_register_callback(conn->conn, FC_CHAT_USER_NICKCHANGED,	_firebind_chat_NICK);
		firetalk_register_callback(conn->conn, FC_CHAT_LISTMEMBER,		_firebind_chat_NAMES);

		firetalk_register_callback(conn->conn, FC_FILE_OFFER,			_firebind_file_offer);
		firetalk_register_callback(conn->conn, FC_FILE_START,			_firebind_file_start);
		firetalk_register_callback(conn->conn, FC_FILE_PROGRESS,		_firebind_file_progress);
		firetalk_register_callback(conn->conn, FC_FILE_FINISH,			_firebind_file_finish);
		firetalk_register_callback(conn->conn, FC_FILE_ERROR,			_firebind_file_error);

		firetalk_register_callback(conn->conn, FC_NEEDPASS,			_firebind_needpass);

#if 0
		firetalk_subcode_register_request_callback(conn->conn, "VERSION",	_firebind_ctcp_VERSION);
		firetalk_subcode_register_request_callback(conn->conn, "PING",		_firebind_ctcp_PING);
		firetalk_subcode_register_request_callback(conn->conn, "LC",		_firebind_ctcp_LC);
		firetalk_subcode_register_request_callback(conn->conn, "HEXTEXT",	_firebind_ctcp_HEXTEXT);
		firetalk_subcode_register_request_callback(conn->conn, "AUTOPEER",	_firebind_ctcp_AUTOPEER);
		firetalk_subcode_register_request_callback(conn->conn, NULL,		_firebind_ctcp_default);

		firetalk_subcode_register_reply_callback(conn->conn, "HEXTEXT",		_firebind_ctcprep_HEXTEXT);
		firetalk_subcode_register_reply_callback(conn->conn, "VERSION",		_firebind_ctcprep_VERSION);
		firetalk_subcode_register_reply_callback(conn->conn, "AWAY",		_firebind_ctcprep_AWAY);
		firetalk_subcode_register_reply_callback(conn->conn, NULL,		_firebind_ctcprep_default);
#endif
	}

	return(conn);
}
