/*  _ __   __ _ ___ __  __
** | '_ \ / _` |_ _|  \/  | naim
** | | | | (_| || || |\/| | Copyright 1998-2006 Daniel Reed <n@ml.org>
** |_| |_|\__,_|___|_|  |_| ncurses-based chat client
*/
#ifndef naim_h
#define naim_h	1

#include <ctype.h>
#include <time.h>
#include <stdarg.h>
#include <stdio.h>

#include <firetalk.h>

#define CONIO_MAXPARMS	10

enum {
	cEVENT,
	cEVENT_ALT,
	cTEXT,
	cSELF,
	cBUDDY,
	cBUDDY_WAITING,
	cBUDDY_ADDRESSED,
	cBUDDY_IDLE,
	cBUDDY_AWAY,
	cBUDDY_OFFLINE,
	cBUDDY_QUEUED,
	cBUDDY_TAGGED,
	cBUDDY_FAKEAWAY,
	cBUDDY_TYPING,
	NUMEVENTS
};

enum {
	cINPUT,
	cWINLIST,
	cWINLISTHIGHLIGHT,
	cCONN,
	cIMWIN,
	cSTATUSBAR,
	NUMWINS
};

enum {
	CH_NONE = 0,
	IM_MESSAGE = (1 << 0),
	CH_ATTACKED = (1 << 1),
	CH_ATTACKS = (1 << 2),
	CH_MESSAGE = (1 << 3),
	CH_MISC = (1 << 4),
	CH_USERS = (1 << 5),
	CH_UNKNOWN6 = (1 << 6),
	CH_UNKNOWN7 = (1 << 7),
};

enum {
	ECHOSTATUS_NONE = 0,
	ALSO_STATUS = (1 << 0),
	ALWAYS_STATUS = (1 << 1),
	NOLOG = (1 << 2),
};

enum {
	RF_NONE = 0,
	RF_AUTOMATIC = (1 << 0),
	RF_ACTION = (1 << 1),
	RF_NOLOG = (1 << 2),
	RF_ENCRYPTED = (1 << 3),
	RF_CHAT = (1 << 4),
};

enum {
	COLOR_HONOR_USER,
	COLOR_FORCE_OFF,
	COLOR_FORCE_ON,
};

typedef struct {
	char	*name;
	unsigned char gotaway:1;
} awayar_t;

typedef struct {
	char	*buf;
	int	len;
	unsigned char foundfirst:1,
		foundmatch:1,
		foundmult:1;
} namescomplete_t;

typedef struct buddylist_ts {
	char	*_account,
		*_group,
		*_name,
		*crypt,
		*tzname,
		*tag,
		*caps;
	struct buddylist_ts *next;
	long	warnval;
	int	peer,
		typing;
	unsigned char offline:1,
		isaway:1,
		isidle:1;	// is the buddy idle for more than some threshhold?
} buddylist_t;
#define DEFAULT_GROUP	"User"
#define CHAT_GROUP	"Chat"
#define USER_ACCOUNT(x)	((x)->_account)
#define USER_NAME(x)	(((x)->_name != NULL)?(x)->_name:(x)->_account)
#define USER_GROUP(x)	((x)->_group)
#define USER_PERMANENT(x) (strcasecmp((x)->_group, DEFAULT_GROUP) != 0)

typedef struct {
	char	*key;
	struct {
		unsigned char *line;
		char	*name;
		int	reps;
		time_t	lasttime;
		int	flags;
		unsigned char istome:1;
	} last;
	unsigned char isoper:1,	// chat operator
		isaddressed:1,	// message addressed sent to me
		offline:1;
} chatlist_t;

typedef struct firetalk_useragent_transfer_t {
	struct firetalk_transfer_t *handle;
	char	*from,
		*remote,
		*local;
	long	size,
		bytes;
	double	started;
	time_t	lastupdate;
	struct buddywin_t *bwin;
} transfer_t;

typedef enum {
	CHAT,
	BUDDY,
	TRANSFER,
} et_t;

typedef struct {
	void	*win;
	FILE	*logfile;
	int	height;
	unsigned char dirty:1,
		small:1;
} win_t;

typedef struct buddywin_t {
	char	*winname,
		*blurb;
	unsigned char waiting:1,/* text waiting to be read (overrides
				** offline and isbuddy in bupdate())
				*/
		keepafterso:1;	/* keep window open after buddy signs off? */
	win_t	nwin;
	char	**pouncear;
	int	pouncec;
	time_t	informed,
		closetime;
	double	viewtime;
	union {
		buddylist_t	*buddy;
		chatlist_t	*chat;
		transfer_t	*transfer;
	} e;
	et_t	et;
	struct buddywin_t *next;
} buddywin_t;

typedef struct ignorelist_ts {
	char	*screenname,
		*notes;
	struct ignorelist_ts *next;
	time_t	informed;
} ignorelist_t;

typedef struct {
	int	f[NUMEVENTS],
		b[NUMWINS];
	struct {
		int	startx, starty,
			widthx, widthy, pady;
	} wstatus, winput, winfo, waway, wtextedit;
} faimconf_t;
#define nw_COLORS	8
#define C(back, fore)	(nw_COLORS*faimconf.b[c ## back] +            faimconf.f[c ## fore])
#define CI(back, fore)	(          faimconf.b[c ## back] + nw_COLORS*(faimconf.f[c ## fore]%COLOR_PAIRS))
#define CB(back, fore)	(nw_COLORS*faimconf.b[c ## back] +            faimconf.b[c ## fore])

typedef struct firetalk_useragent_connection_t {
	char	*sn,
		*password,
		*winname,
		*server,
		*profile;
	long	warnval;
	int	port, proto;
	time_t	online;
	double	lastupdate, lag;
	struct firetalk_connection_t *conn;
	FILE	*logfile;
	win_t	nwin;
	buddylist_t *buddyar;
	ignorelist_t *idiotar;
	buddywin_t *curbwin;
	struct firetalk_useragent_connection_t *next;
} conn_t;

typedef struct {
	char	*name,
		*script;
} alias_t;

typedef struct {
	const char *var,
		*val,
		*desc;
} rc_var_s_t;

typedef struct {
	const char *var;
	long	val;
	const char *desc;
} rc_var_i_t;

typedef struct {
	char	*from,
		*replace;
} html_clean_t;


/*
 * Provide G_GNUC_INTERNAL that is used for marking library functions
 * as being used internally to the lib only, to not create inefficient PLT entries
 */
#if defined (__GNUC__)
# define G_GNUC_INTERNAL	__attribute((visibility("hidden")))
#else
# define G_GNUC_INTERNAL
#endif




/* alias.c */
void	alias_makealias(const char *, const char *);
int	alias_doalias(const char *, const char *);

/* atomizer.c */
char	*firstatom(char *string, char *bounds) G_GNUC_INTERNAL;
char	*firstwhite(char *string) G_GNUC_INTERNAL;
char	*atom(char *string) G_GNUC_INTERNAL;

/* buddy.c */
const unsigned char *naim_normalize(const unsigned char *const name) G_GNUC_INTERNAL;
void	playback(conn_t *const conn, buddywin_t *const, const int) G_GNUC_INTERNAL;
void	bnewwin(conn_t *conn, const char *, et_t);
void	bupdate(void);
void	bcoming(conn_t *conn, const char *) G_GNUC_INTERNAL;
void	bgoing(conn_t *conn, const char *) G_GNUC_INTERNAL;
void	bidle(conn_t *conn, const char *, int) G_GNUC_INTERNAL;
void	baway(conn_t *conn, const char *, int) G_GNUC_INTERNAL;
void	verify_winlist_sanity(conn_t *const conn, const buddywin_t *const verifywin) G_GNUC_INTERNAL;
void	bclose(conn_t *conn, buddywin_t *bwin, int _auto);
buddywin_t *bgetwin(conn_t *conn, const char *, et_t);
buddywin_t *bgetanywin(conn_t *conn, const char *);
buddywin_t *bgetbuddywin(conn_t *conn, const buddylist_t *);
void	bclearall(conn_t *conn, int) G_GNUC_INTERNAL;
void	naim_changetime(void) G_GNUC_INTERNAL;

/* conio.c */
void	naim_eval(const char *);
const char *conio_valid(const char *cmd, conn_t *conn, const int argc);
void	conio_handlecmd(const char *);
void	conio_handleline(const char *line);
void	gotkey(int) G_GNUC_INTERNAL;
void	conio_hook_init(void) G_GNUC_INTERNAL;

/* echof.c */
void	status_echof(conn_t *conn, const unsigned char *format, ...);
void	window_echof(buddywin_t *bwin, const unsigned char *format, ...);
void	echof(conn_t *conn, const unsigned char *where, const unsigned char *format, ...);

/* events.c */
void	updateidletime(void) G_GNUC_INTERNAL;
void	events_hook_init(void) G_GNUC_INTERNAL;

/* fireio.c */
void	chat_flush(buddywin_t *bwin) G_GNUC_INTERNAL;
conn_t	*naim_newconn(int);
void	naim_set_info(conn_t *conn, const char *) G_GNUC_INTERNAL;
void	naim_lastupdate(conn_t *conn) G_GNUC_INTERNAL;
buddywin_t *cgetwin(conn_t *, const char *);
void	naim_chat_listmembers(conn_t *conn, const char *const chat) G_GNUC_INTERNAL;
void	fremove(transfer_t *) G_GNUC_INTERNAL;
transfer_t *fnewtransfer(struct firetalk_transfer_t *handle, buddywin_t *bwin, const char *filename,
		const char *from, long size) G_GNUC_INTERNAL;
void	fireio_hook_init(void) G_GNUC_INTERNAL;
void	naim_awaylog(conn_t *conn, const char *src, const char *msg) G_GNUC_INTERNAL;
void	naim_setversion(conn_t *conn) G_GNUC_INTERNAL;

/* hamster.c */
void	logim(conn_t *conn, const char *source, const char *target, const unsigned char *message) G_GNUC_INTERNAL;
void	naim_send_im(conn_t *conn, const char *, const char *, const int _auto);
void	naim_send_im_away(conn_t *conn, const char *, const char *, int force);
void	naim_send_act(conn_t *const conn, const char *const, const unsigned char *const);
void	hamster_hook_init(void) G_GNUC_INTERNAL;
void	setaway(const int auto_flag);
void	unsetaway(void);
void	sendaway(conn_t *conn, const char *);
int	getvar_int(conn_t *conn, const char *);
char	*getvar(conn_t *conn, const char *);

/* helpcmd.c */
void	help_printhelp(const char *) G_GNUC_INTERNAL;

/* hwprintf.c */
int	vhwprintf(win_t *, int, const unsigned char *, va_list);
int	hwprintf(win_t *, int, const unsigned char *, ...);

/* rc.c */
const char *account_tabcomplete(conn_t *const conn, const char *start, const char *buf, const int bufloc, int *const match, const char **desc) G_GNUC_INTERNAL;
const char *buddy_tabcomplete(conn_t *const conn, const char *start, const char *buf, const int bufloc, int *const match, const char **desc) G_GNUC_INTERNAL;
const char *idiot_tabcomplete(conn_t *const conn, const char *start, const char *buf, const int bufloc, int *const match, const char **desc) G_GNUC_INTERNAL;
buddylist_t *rgetlist(conn_t *, const char *);
buddylist_t *rgetlist_friendly(conn_t *conn, const char *friendly);
buddylist_t *raddbuddy(conn_t *, const char *, const char *, const char *);
void	do_delbuddy(buddylist_t *b);
void	rdelbuddy(conn_t *, const char *);
void	raddidiot(conn_t *, const char *, const char *);
void	rdelidiot(conn_t *, const char *);
int	rc_resize(faimconf_t *) G_GNUC_INTERNAL;
void	rc_initdefs(faimconf_t *) G_GNUC_INTERNAL;
int	naim_read_config(const char *) G_GNUC_INTERNAL;

/* rodents.c */
int	aimcmp(const unsigned char *, const unsigned char *) G_GNUC_INTERNAL;
int	aimncmp(const unsigned char *, const unsigned char *, int len) G_GNUC_INTERNAL;
const char *dtime(double t) G_GNUC_INTERNAL;
const char *dsize(double b) G_GNUC_INTERNAL;
void	htmlstrip(char *bb);
void	htmlreplace(char *bb, char what);

/* set.c */
const char *set_tabcomplete(conn_t *const conn, const char *start, const char *buf, const int bufloc, int *const match, const char **desc) G_GNUC_INTERNAL;
void	set_echof(const char *const format, ...);
void	set_setvar(const char *, const char *) G_GNUC_INTERNAL;

/* win.c */
void	do_resize(conn_t *conn, buddywin_t *bwin) G_GNUC_INTERNAL;
void	statrefresh(void) G_GNUC_INTERNAL;
void	whidecursor(void) G_GNUC_INTERNAL;
void	wsetup(void) G_GNUC_INTERNAL;
void	wshutitdown(void) G_GNUC_INTERNAL;
void	win_resize(void) G_GNUC_INTERNAL;
int	nw_printf(win_t *win, int, int, const unsigned char *, ...) G_GNUC_INTERNAL;
int	nw_titlef(const unsigned char *, ...) G_GNUC_INTERNAL;
int	nw_statusbarf(const unsigned char *format, ...) G_GNUC_INTERNAL;
void	nw_initwin(win_t *win, int bg) G_GNUC_INTERNAL;
void	nw_erase(win_t *win) G_GNUC_INTERNAL;
void	nw_refresh(win_t *win) G_GNUC_INTERNAL;
void	nw_attr(win_t *win, char B, char I, char U, char EM,
		char STRONG, char CODE) G_GNUC_INTERNAL;
void	nw_color(win_t *win, int pair) G_GNUC_INTERNAL;
void	nw_flood(win_t *win, int pair) G_GNUC_INTERNAL;
void	nw_addch(win_t *win, const unsigned long ch) G_GNUC_INTERNAL;
void	nw_addstr(win_t *win, const unsigned char *) G_GNUC_INTERNAL;
void	nw_move(win_t *win, int row, int col) G_GNUC_INTERNAL;
void	nw_delwin(win_t *win) G_GNUC_INTERNAL;
void	nw_touchwin(win_t *win) G_GNUC_INTERNAL;
void	nw_newwin(win_t *win) G_GNUC_INTERNAL;
void	nw_hline(win_t *win, unsigned long ch, int row) G_GNUC_INTERNAL;
void	nw_vline(win_t *win, unsigned long ch, int col) G_GNUC_INTERNAL;
void	nw_mvwin(win_t *win, int row, int col) G_GNUC_INTERNAL;
void	nw_resize(win_t *win, int row, int col) G_GNUC_INTERNAL;
int	nw_getcol(win_t *win) G_GNUC_INTERNAL;
int	nw_getrow(win_t *win) G_GNUC_INTERNAL;
void	nw_getline(win_t *win, char *buf, int buflen) G_GNUC_INTERNAL;
int	nw_getch(void) G_GNUC_INTERNAL;
void	nw_getpass(win_t *win, char *pass, int len) G_GNUC_INTERNAL;

#endif /* naim_h */
