/*  _ __   __ _ ___ __  __
** | '_ \ / _` |_ _|  \/  | naim
** | | | | (_| || || |\/| | Copyright 1998-2006 Daniel Reed <n@ml.org>
** |_| |_|\__,_|___|_|  |_| ncurses-based chat client
*/
#include <naim/naim.h>
#include <naim/modutil.h>
#include <pwd.h>
#include <sys/utsname.h>

#include "naim-int.h"
#include "snapshot.h"
#include "cmdar.h"

extern conn_t	*curconn;
extern faimconf_t faimconf;
extern time_t	now, awaytime;
extern double	nowf;
extern char	*sty, *statusbar_text;

extern int awayc G_GNUC_INTERNAL;
extern awayar_t *awayar G_GNUC_INTERNAL;
int	awayc = 0;
awayar_t *awayar = NULL;

#define NAIM_VERSION_STRING	"naim:" PACKAGE_VERSION NAIM_SNAPSHOT
static char naim_version[1024];

static int fireio_proto_newnick(void *userdata, conn_t *conn, const char *newnick) {
	STRREPLACE(conn->sn, newnick);

	script_setvar("SN", newnick);

	return(HOOK_CONTINUE);
}

static int fireio_proto_nickchanged(void *userdata, conn_t *conn, const char *oldnick, const char *newnick) {
	buddywin_t *bwin;
	buddylist_t *buddy = conn->buddyar;

	if ((buddy = rgetlist(conn, oldnick)) == NULL)
		return(HOOK_CONTINUE);
	if (strcmp(buddy->_account, newnick) != 0) {
		script_hook_changebuddy(buddy, newnick);
		STRREPLACE(buddy->_account, newnick);
	}

	if ((bwin = bgetbuddywin(conn, buddy)) != NULL) {
		window_echof(bwin, "<font color=\"#00FFFF\">%s</font> is now known as <font color=\"#00FFFF\">%s</font>.\n",
			oldnick, newnick);
		STRREPLACE(bwin->winname, newnick);
		bupdate();
	}

	return(HOOK_CONTINUE);
}

#define STANDARD_TRAILER	\
	"&nbsp;<br>"		\
	"Visit <a href=\"http://naim.n.ml.org/\">http://naim.n.ml.org/</a> for the latest naim."

void	naim_set_info(conn_t *conn, const char *str) {
	const char *defar[] = {
"Tepid! Tepid is no good for a star, but it'll do for stardust.<br>\n"
			STANDARD_TRAILER,
"Resisting temptation is easier when you think you'll probably get "
"another chance later on.<br>\n"
			STANDARD_TRAILER,
"A little inaccuracy sometimes saves tons of explanation.<br>\n"
"&nbsp; &nbsp; &nbsp; &nbsp; -- H. H. Munroe<br>\n"
			STANDARD_TRAILER,
"What have you done today to make you feel proud?<br>\n"
			STANDARD_TRAILER,
"<a href=\"http://creativecommons.org/\">Creativity always builds on the past.</a><br>\n"
			STANDARD_TRAILER,
"Those who make peaceful revolution impossible will make violent "
"revolution inevitable.<br>\n"
"&nbsp; &nbsp; &nbsp; &nbsp; -- John F. Kennedy<br>\n"
			STANDARD_TRAILER,
"The question of whether computers can think is just like the question of whether submarines can swim.<br>\n"
"&nbsp; &nbsp; &nbsp; &nbsp; -- Edsger W. Dijkstra<br>\n"
			STANDARD_TRAILER,
"People sleep peaceably in their beds at night only because "
"rough men stand ready to do violence on their behalf.<br>\n"
			STANDARD_TRAILER,
"If you can't fix it, you gotta stand it.<br>\n"
			STANDARD_TRAILER,
"While you don't greatly need the outside world, it's still very "
"reassuring to know that it's still there.<br>\n"
			STANDARD_TRAILER,
	};

	if (str != NULL)
		firetalk_set_info(conn->conn, str);
	else {
		int	d;

		d = rand()%sizeof(defar)/sizeof(*defar);
		firetalk_set_info(conn->conn, defar[d]);
	}
}

static int fireio_postselect(void *userdata, fd_set *rfd, fd_set *wfd, fd_set *efd) {
	struct timeval tv;
	char	buf[1024];

	gettimeofday(&tv, NULL);
	now = tv.tv_sec;
	nowf = tv.tv_usec/1000000. + ((double)now);
	snprintf(buf, sizeof(buf), "%lu", now);
	script_setvar("nowi", buf);

	return(HOOK_CONTINUE);
}

void	naim_setversion(conn_t *conn) {
	const char *where,
		*where2,
		*term,
		*lang;
	struct utsname unbuf;

	if ((where = getenv("DISPLAY")) != NULL) {
		if ((strncmp(where, ":0", 2) == 0) && (getenv("SSH_CLIENT") == NULL))
			where = " xterm";
		else
			where = " ssh -X";
	} else {
		if (getenv("SSH_CLIENT") != NULL)
			where = " ssh";
		else
			where = "";
	}
	if (sty != NULL)
		where2 = " screen";
	else
		where2 = "";
	if ((term = getenv("TERM")) == NULL)
		term = "(unknown terminal)";
	if ((lang = getenv("LANG")) == NULL)
		lang = "(default language)";

	if (uname(&unbuf) == 0)
		snprintf(naim_version, sizeof(naim_version), "%s:%s %s %s, %ix%i%s%s, %s %s", NAIM_VERSION_STRING,
			unbuf.sysname, unbuf.release, unbuf.machine, COLS, LINES, where, where2, term, lang);
	else
		snprintf(naim_version, sizeof(naim_version), "%s:unknown, %ix%i%s%s, %s %s", NAIM_VERSION_STRING,
			COLS, LINES, where, where2, term, lang);

	firetalk_subcode_register_request_reply(conn->conn, "VERSION", naim_version);
}

static int fireio_doinit(void *userdata, conn_t *conn, const char *screenname) {
	naim_setversion(conn);

	STRREPLACE(conn->sn, screenname);

	if (awaytime > 0)
		status_echof(conn, "Updating profile and setting away message...\n");
	else
		status_echof(conn, "Updating profile...\n");

	naim_set_info(conn, conn->profile);

	if (awaytime > 0)
		firetalk_set_away(conn->conn, script_getvar("awaymsg"), 0);

	return(HOOK_CONTINUE);
}

static int fireio_warned(void *userdata, conn_t *conn, int newlev, const char *who) {
	echof(conn, NULL, "<font color=\"#00FFFF\">%s</font> just warned you (%i).\n",
		who, newlev);
	conn->warnval = newlev;

	return(HOOK_CONTINUE);
}

static int fireio_buddy_idle(void *userdata, conn_t *conn, const char *who, long idletime) {
	if (idletime >= 10)
		bidle(conn, who, 1);
	else
		bidle(conn, who, 0);

	return(HOOK_CONTINUE);
}

static int fireio_buddy_eviled(void *userdata, conn_t *conn, const char *who, long warnval) {
	buddylist_t *blist;

	if ((blist = rgetlist(conn, who)) != NULL)
		blist->warnval = warnval;

	return(HOOK_CONTINUE);
}

static int fireio_buddy_capschanged(void *userdata, conn_t *conn, const char *who, const char *caps) {
	buddylist_t *blist;

	if ((blist = rgetlist(conn, who)) != NULL) {
		int	i, j, strtolower = 1;

		if ((blist->caps != NULL) && (blist->caps[0] != 0)) {
			buddywin_t *bwin;

			if ((bwin = bgetbuddywin(conn, blist)) != NULL)
				window_echof(bwin, "Client capabilities for <font color=\"#00FFFF\">%s</font> <font color=\"#800000\">[<B>%s</B>]</font> have changed, possibly meaning %s has signed onto or off with multiple clients.\n",
					user_name(NULL, 0, conn, blist), USER_GROUP(blist), USER_NAME(blist));
		}

		blist->caps = realloc(blist->caps, 2*strlen(caps)+1);

		for (i = 0; (caps[i] != 0) && (caps[i] != ' '); i++)
			if (islower(caps[i])) {
				strtolower = 0;
				break;
			}

		for (j = i = 0; caps[i] != 0; i++)
			if (caps[i] == ' ') {
				int	x;

				strtolower = 1;
				for (x = i+1; (caps[x] != 0) && (caps[x] != ' '); x++)
					if (islower(caps[x])) {
						strtolower = 0;
						break;
					}
				blist->caps[j++] = ',';
				blist->caps[j++] = ' ';
			} else if (caps[i] == '_')
				blist->caps[j++] = ' ';
			else if (strtolower && (j > 0) && (blist->caps[j-1] != ' '))
				blist->caps[j++] = tolower(caps[i]);
			else
				blist->caps[j++] = caps[i];
		blist->caps[j] = 0;
	}

	return(HOOK_CONTINUE);
}

static int fireio_buddy_typing(void *userdata, conn_t *conn, const char *who, int typing) {
	buddylist_t *blist;

	if ((blist = rgetlist(conn, who)) != NULL) {
		if (typing)
			blist->typing = now;
		else
			blist->typing = 0;
	}

	return(HOOK_CONTINUE);
}

static int fireio_buddy_away(void *userdata, conn_t *conn, const char *who) {
	baway(conn, who, 1);

	return(HOOK_CONTINUE);
}

static int fireio_buddy_unaway(void *userdata, conn_t *conn, const char *who) {
	baway(conn, who, 0);

	return(HOOK_CONTINUE);
}

static int fireio_buddyadded(void *userdata, conn_t *conn, const char *screenname, const char *group, const char *friendly) {
	buddylist_t *blist;

	if ((blist = rgetlist(conn, screenname)) == NULL) {
		blist = raddbuddy(conn, screenname, group, friendly);
		if (USER_PERMANENT(blist))
			status_echof(conn, "Added <font color=\"#00FFFF\">%s</font> <font color=\"#800000\">[<B>%s</B>]</font> to your permanent buddy list.\n",
				user_name(NULL, 0, conn, blist), USER_GROUP(blist));
		else
			status_echof(conn, "Added <font color=\"#00FFFF\">%s</font> <font color=\"#800000\">[<B>%s</B>]</font> to your non-permanent buddy list.\n",
				user_name(NULL, 0, conn, blist), USER_GROUP(blist));
	} else {
		if (strcmp(blist->_group, group) != 0)
			status_echof(conn, "Moved <font color=\"#00FFFF\">%s</font> <font color=\"#800000\">[<B>%s</B>]</font> to group <font color=\"#00FFFF\">%s</font>.\n",
				user_name(NULL, 0, conn, blist), USER_GROUP(blist), group);
		else
			status_echof(conn, "Renamed <font color=\"#00FFFF\">%s</font> <font color=\"#800000\">[<B>%s</B>]</font> to <font color=\"#FF0000\">%s</font>.\n",
				user_name(NULL, 0, conn, blist), USER_GROUP(blist), (friendly != NULL)?friendly:USER_ACCOUNT(blist));
	}

	STRREPLACE(blist->_group, group);
	if (friendly != NULL)
		STRREPLACE(blist->_name, friendly);
	else
		FREESTR(blist->_name);

	return(HOOK_CONTINUE);
}

static int fireio_buddyremoved(void *userdata, conn_t *conn, const char *screenname) {
	buddywin_t *bwin;
	buddylist_t *blist;

	if ((bwin = bgetwin(conn, screenname, BUDDY)) != NULL) {
		blist = bwin->e.buddy;
		status_echof(conn, "Closed window <font color=\"#00FFFF\">%s</font> due to buddy removal.\n",
			bwin->winname);
		bclose(conn, bwin, 0);
	} else
		blist = rgetlist(conn, screenname);

	if (blist != NULL) {
		status_echof(conn, "Removed <font color=\"#00FFFF\">%s</font> <font color=\"#800000\">[<B>%s</B>]</font> from your buddy list.\n",
			user_name(NULL, 0, conn, blist), USER_GROUP(blist));
		rdelbuddy(conn, screenname);
	}

	return(HOOK_CONTINUE);
}

static int fireio_denyadded(void *userdata, conn_t *conn, const char *screenname) {
	status_echof(conn, "Added <font color=\"#00FFFF\">%s</font> to your block list.\n", screenname);

	raddidiot(conn, screenname, "block");

	return(HOOK_CONTINUE);
}

static int fireio_denyremoved(void *userdata, conn_t *conn, const char *screenname) {
	status_echof(conn, "Removed <font color=\"#00FFFF\">%s</font> from your block list.\n", screenname);

	rdelidiot(conn, screenname);

	return(HOOK_CONTINUE);
}

static int fireio_buddy_coming(void *userdata, conn_t *conn, const char *who) {
	bcoming(conn, who);

	return(HOOK_CONTINUE);
}

static int fireio_buddy_going(void *userdata, conn_t *conn, const char *who) {
	bgoing(conn, who);

	return(HOOK_CONTINUE);
}

static int fireio_recvfrom_ignorelist(void *userdata, conn_t *conn, char **name, char **dest, unsigned char **message, int *len, int *flags) {
	ignorelist_t *ig;

	if ((*dest != NULL) && (strcmp(*dest, ":RAW") == 0) && (getvar_int(conn, "showraw") == 0))
		return(HOOK_STOP);

	for (ig = conn->idiotar; ig != NULL; ig = ig->next)
		if (firetalk_compare_nicks(conn->conn, *name, ig->screenname) == FE_SUCCESS) {
			if (*dest == NULL)
				firetalk_im_evil(conn->conn, *name);
			return(HOOK_STOP);
		}

	return(HOOK_CONTINUE);
}

static int fireio_recvfrom_decrypt(void *userdata, conn_t *conn, char **name, char **dest, unsigned char **message, int *len, int *flags) {
	if ((*dest == NULL) && !(*flags & RF_ACTION)) {
		buddylist_t *blist = rgetlist(conn, *name);

		if ((blist != NULL) && (blist->crypt != NULL) && ((blist->peer <= 3) || (*flags & RF_ENCRYPTED))) {
			int	i, j = 0;

			for (i = 0; i < *len; i++) {
				(*message)[i] = (*message)[i] ^ (unsigned char)blist->crypt[j++];
				if (blist->crypt[j] == 0)
					j = 0;
			}
			if ((*message)[i] != 0) {
				echof(conn, "fireio_recvfrom_decrypt", "Invalid message: len=%i, i=%i, msg[i]=%i\n",
					*len, i, (*message)[i]);
				return(HOOK_STOP);
			}
		}
	}

	return(HOOK_CONTINUE);
}

static int fireio_recvfrom_log(void *userdata, conn_t *conn, char **name, char **dest, unsigned char **message, int *len, int *flags) {
	if (!(*flags & RF_NOLOG))
		logim(conn, *name, *dest, *message);

	return(HOOK_CONTINUE);
}

static int fireio_recvfrom_beep(void *userdata, conn_t *conn, char **name, char **dest, unsigned char **message, int *len, int *flags) {
	if (*dest == NULL) {
		int	beeponim = getvar_int(conn, "beeponim");

		if ((beeponim > 1) || ((awaytime == 0) && (beeponim == 1)))
			beep();
	}

	return(HOOK_CONTINUE);
}

static int fireio_recvfrom_autobuddy(void *userdata, conn_t *conn, char **name, char **dest, unsigned char **message, int *len, int *flags) {
	if (*dest == NULL) {
		buddylist_t *blist = rgetlist(conn, *name);

		if (getvar_int(conn, "autobuddy")) {
			if (blist == NULL) {
				assert(bgetwin(conn, *name, BUDDY) == NULL);
				status_echof(conn, "Adding <font color=\"#00FFFF\">%s</font> to your buddy list due to autobuddy.\n", *name);
				blist = raddbuddy(conn, *name, DEFAULT_GROUP, NULL);
				assert(blist->offline == 1);
				bnewwin(conn, *name, BUDDY);
				firetalk_im_add_buddy(conn->conn, *name, USER_GROUP(blist), NULL);
			} else if (bgetbuddywin(conn, blist) == NULL) {
				buddywin_t *bwin;

				bnewwin(conn, *name, BUDDY);
				bwin = bgetbuddywin(conn, blist);
				window_echof(bwin, "<font color=\"#00FFFF\">%s</font> <font color=\"#800000\">[<B>%s</B>]</font> is still online...\n",
					user_name(NULL, 0, conn, blist), USER_GROUP(blist));
			}
		}
	}

	return(HOOK_CONTINUE);
}

static int fireio_recvfrom_display_user(void *userdata, conn_t *conn, char **name, char **dest, unsigned char **message, int *len, int *flags) {
	buddylist_t *blist;
	buddywin_t *bwin;

	if (*dest != NULL)
		return(HOOK_CONTINUE);

	if (((blist = rgetlist(conn, *name)) == NULL) || ((bwin = bgetbuddywin(conn, blist)) == NULL)) {
		const char *format;

		naim_lastupdate(conn);

		if (*flags & RF_ACTION) {
			if ((blist != NULL) && (blist->crypt != NULL))
				format = "<B>*&gt; %s</B>";
			else
				format = "*&gt; <B>%s</B>";
		} else if (*flags & RF_AUTOMATIC) {
			if ((blist != NULL) && (blist->crypt != NULL))
				format = "<B>-%s-</B>";
			else
				format = "-<B>%s</B>-";
		} else {
			if ((blist != NULL) && (blist->crypt != NULL))
				format = "<B>[%s]</B>";
			else
				format = "[<B>%s</B>]";
		}
		WINTIME(&(conn->nwin), CONN);
		/* Use name here instead of USER_NAME(blist) even if blist is
		** not NULL, since these messages are going to the status
		** window, and we want them to remain unambiguous.
		*/
		hwprintf(&(conn->nwin), C(CONN,BUDDY), format, *name);
		hwprintf(&(conn->nwin), C(CONN,TEXT), "%s<body>%s</body><br>", (strncmp(*message, "'s ", 3) == 0)?"":" ", *message);
	} else {
		const char *format;

		assert(blist != NULL);
		assert(bwin->et == BUDDY);
		assert(bwin->e.buddy == blist);

		if (getvar_int(conn, "chatter") & IM_MESSAGE)
			bwin->waiting = 1;
		bwin->keepafterso = 1;

		if (*flags & RF_ACTION) {
			if (blist->crypt != NULL)
				format = "<B>* %s</B>";
			else
				format = "* <B>%s</B>";
		} else if (*flags & RF_AUTOMATIC) {
			if (blist->crypt != NULL)
				format = "<B>-%s-</B>";
			else
				format = "-<B>%s</B>-";
		} else {
			if (blist->crypt != NULL)
				format = "<B>%s:</B>";
			else
				format = "<B>%s</B>:";
		}

		WINTIME(&(bwin->nwin), IMWIN);
		hwprintf(&(bwin->nwin), C(IMWIN,BUDDY), format, USER_NAME(blist));
		hwprintf(&(bwin->nwin), C(IMWIN,TEXT), "%s<body>%s</body><br>", (strncmp(*message, "'s ", 3) == 0)?"":" ", *message);

		if (!(*flags & RF_AUTOMATIC)) {
			int	autoreply = getvar_int(conn, "autoreply");

			if ((autoreply > 0) && (awaytime > 0) && (*script_getvar("awaymsg") != 0)
				&& ((now - bwin->informed) > 60*autoreply)
				&& (firetalk_compare_nicks(conn->conn, *name, conn->sn) != FE_SUCCESS)) {
				int	autoaway = script_getvar_int("autoaway"),
					idletime = script_getvar_int("idletime");

				if ((autoaway == 0) || (idletime >= (now-awaytime)/60) || (idletime >= 10)) {
					sendaway(conn, *name);
					bwin->informed = now;
				}
			}
		}
	}
	bupdate();

	return(HOOK_CONTINUE);
}

static void fireio_recvfrom_display_chat_print(void *userdata, buddywin_t *bwin, const int flags, const int istome, const char *name, const char *prefix, const unsigned char *message) {
	const char *format;

	if (prefix == NULL)
		prefix = "&gt;";

	if (flags & RF_ACTION)
		format = "* <B>%s</B>";
	else if (flags & RF_AUTOMATIC)
		format = "-<B>%s</B>-";
	else
		format = "&lt;<B>%s</B>%s";
	hwprintf(&(bwin->nwin), istome?C(IMWIN,BUDDY_ADDRESSED):C(IMWIN,BUDDY), 
		format, name, prefix);

	hwprintf(&(bwin->nwin), C(IMWIN,TEXT), " <body>%s%s%s</body><br>",
		istome?"<B>":"", message, istome?"</B>":"");
}

void	chat_flush(buddywin_t *bwin) {
	assert(bwin->et == CHAT);
	assert(bwin->e.chat->last.reps >= 0);
	if (bwin->e.chat->last.reps > 0) {
		if (bwin->e.chat->last.reps == 1) {
			assert(bwin->e.chat->last.lasttime != 0);
			WINTIME_NOTNOW(&(bwin->nwin), IMWIN, bwin->e.chat->last.lasttime);
			fireio_recvfrom_display_chat_print(NULL, bwin, bwin->e.chat->last.flags, bwin->e.chat->last.istome, bwin->e.chat->last.name, NULL, bwin->e.chat->last.line);
		} else {
			assert(bwin->e.chat->last.lasttime != 0);
			WINTIME_NOTNOW(&(bwin->nwin), IMWIN, bwin->e.chat->last.lasttime);
			hwprintf(&(bwin->nwin), C(IMWIN,TEXT), "<B>[Last message repeated %i more times]</B><br>", bwin->e.chat->last.reps);
		}
		bwin->e.chat->last.reps = 0;
	}
	FREESTR(bwin->e.chat->last.line);
	FREESTR(bwin->e.chat->last.name);
}

static int fireio_recvfrom_display_chat(void *userdata, conn_t *conn, char **name, char **dest, unsigned char **message, int *len, int *flags) {
	buddywin_t *bwin;
	int	istome;
	char	*prefix = NULL;
	unsigned char *message_save;

	if (*dest == NULL)
		return(HOOK_CONTINUE);

	{
		unsigned char *match;

		if (conn->sn == NULL)
			istome = 0;
		else if ((aimncmp(*message, conn->sn, strlen(conn->sn)) == 0) && !isalpha(*(*message+strlen(conn->sn))))
			istome = 1;
		else if (((match = strstr(*message, conn->sn)) != NULL)
			&& ((match == *message) || !isalpha(*(match-1)))
			&& !isalpha(*(match+strlen(conn->sn))))
			istome = 1;
		else
			istome = 0;
	}

	bwin = cgetwin(conn, *dest);
	assert(bwin->et == CHAT);
	if (getvar_int(conn, "chatter") & CH_MESSAGE) {
		bwin->waiting = 1;
		if (istome)
			bwin->e.chat->isaddressed = 1;
	}

	if (bwin->e.chat->last.line != NULL)
		assert(bwin->e.chat->last.name != NULL);
	if ((bwin->e.chat->last.line != NULL) && (*flags == bwin->e.chat->last.flags)
		&& (firetalk_compare_nicks(conn->conn, *name, bwin->e.chat->last.name) == FE_SUCCESS)
		&& (strcasecmp(*message, bwin->e.chat->last.line) == 0)) {
		bwin->e.chat->last.reps++;
		bwin->e.chat->last.lasttime = now;
		return(HOOK_CONTINUE);
	}
	if (bwin->e.chat->last.reps > 0)
		chat_flush(bwin);
	assert(bwin->e.chat->last.reps == 0);

	message_save = strdup(*message);

	if ((bwin->winname[0] != ':') && (bwin->e.chat->last.line != NULL)) {
		char	*tmp = strdup(*message);
		size_t	off, add;

		htmlreplace(tmp, '_');
		off = strspn(tmp, "_");
		add = strcspn(tmp+off, " ");
		assert(bwin->e.chat->last.name != NULL);
		if ((add > off) && isalpha(tmp[off]) && !isalpha(tmp[off+add-1])) {
			if (strncasecmp(tmp+off, bwin->e.chat->last.name, strlen(bwin->e.chat->last.name)) == 0) {
				static int sent_carat_desc = 0;

				if (!sent_carat_desc) {
					STRREPLACE(statusbar_text, "A ^ near a speaker name indicates that message was addressed to the previous speaker.");
					sent_carat_desc = 1;
				}
				prefix = "^";
				memmove((*message)+off, (*message)+off+add, strlen((*message)+off+add)+1);
			} else if (strncasecmp(*message, bwin->e.chat->last.line, off+add) == 0) {
				static int sent_plus_desc = 0;

				if (!sent_plus_desc) {
					STRREPLACE(statusbar_text, "A + near a speaker name indicates that message was addressed to the same person as the previous message.");
					sent_plus_desc = 1;
				}
				if (firetalk_compare_nicks(conn->conn, *name, bwin->e.chat->last.name) != FE_SUCCESS)
					prefix = "+";
				memmove((*message)+off, (*message)+off+add, strlen((*message)+off+add)+1);
			}
		}
		free(tmp);
	}

	WINTIME(&(bwin->nwin), IMWIN);
	fireio_recvfrom_display_chat_print(NULL, bwin, *flags, istome, *name, prefix, *message);

	free(bwin->e.chat->last.line);
	bwin->e.chat->last.line = message_save;
	STRREPLACE(bwin->e.chat->last.name, (*name));
	bwin->e.chat->last.reps = 0;
	bwin->e.chat->last.lasttime = now;
	bwin->e.chat->last.flags = *flags;
	bwin->e.chat->last.istome = istome;

	bupdate();

	return(HOOK_CONTINUE);
}

static int fireio_connected(void *userdata, conn_t *conn) {
	buddywin_t *bwin = conn->curbwin;

	if (conn->online > 0) {
		status_echof(conn, "naim just received notification that you have connected to %s at %lu,"
			" but I'm pretty sure you've been connected since %lu."
			" This is a bug, and your session may be unstable.\n",
			conn->winname, now, conn->online);
		return(HOOK_CONTINUE);
	}

	echof(conn, NULL, "You are now connected.\n");

	conn->online = now;
	if (bwin != NULL)
		do {
			if ((bwin->et == CHAT) && (*(bwin->winname) != ':')) {
				char	buf[1024], *name = buf;

				if (bwin->e.chat->key != NULL)
					snprintf(buf, sizeof(buf), "%s %s", bwin->winname, bwin->e.chat->key);
				else
					name = bwin->winname;
				firetalk_chat_join(conn->conn, name);
			}
		} while ((bwin = bwin->next) != conn->curbwin);

	return(HOOK_CONTINUE);
}

static const char naim_tolower_first(const char *const str) {
	if ((*str != 0) && !isupper(str[1]))
		return(tolower(*str));
	return(*str);
}

static int fireio_connectfailed(void *userdata, conn_t *conn, int err, const char *reason) {
	if (reason != NULL)
		echof(conn, "CONNECT", "Unable to connect to %s: %s, %c%s.\n",
			firetalk_strprotocol(conn->proto),
			firetalk_strerror(err), naim_tolower_first(reason), reason+1);
	else
		echof(conn, "CONNECT", "Unable to connect to %s: %s.\n",
			firetalk_strprotocol(conn->proto),
			firetalk_strerror(err));

	if (err == FE_BADUSER) {
		char	*str;

		echof(conn, NULL, "Attempting to reconnect using a different name...\n");
		str = malloc(strlen(conn->sn)+2);
		strcpy(str, conn->sn);
		if (strlen(conn->sn) > 8) {
			int	pos = rand()%(strlen(conn->sn)-1)+1;

			if (str[pos] == '_')
				str[pos] = rand()%9+'1';
			else
				str[pos] = '_';
		} else
			strcat(str, "_");
		free(conn->sn);
		conn->sn = str;
		ua_connect(conn, 0, NULL);
	}

	return(HOOK_CONTINUE);
}

static int fireio_error_msg(void *userdata, conn_t *conn, int error, const char *target, const char *desc) {
	buddywin_t *bwin;

	if ((error == FE_MESSAGETRUNCATED) && (awaytime > 0))
		return(HOOK_CONTINUE);;

	if ((target != NULL) && ((bwin = bgetanywin(conn, target)) != NULL)) {
		if (desc != NULL)
			window_echof(bwin, "ERROR: %s, %c%s\n",
				firetalk_strerror(error), naim_tolower_first(desc), desc+1);
		else
			window_echof(bwin, "ERROR: %s\n",
				firetalk_strerror(error));
		bwin->waiting = 1;
		bupdate();
	} else if (target != NULL) {
		if (desc != NULL)
			status_echof(conn, "ERROR: %s: %s, %c%s\n",
				target, firetalk_strerror(error), naim_tolower_first(desc), desc+1);
		else
			status_echof(conn, "ERROR: %s: %s\n",
				target, firetalk_strerror(error));
	} else {
		if (desc != NULL)
			status_echof(conn, "ERROR: %s, %c%s\n",
				firetalk_strerror(error), naim_tolower_first(desc), desc+1);
		else
			status_echof(conn, "ERROR: %s\n",
				firetalk_strerror(error));
	}

	return(HOOK_CONTINUE);
}

static int fireio_error_disconnect(void *userdata, conn_t *conn, int error) {
	echof(conn, NULL, "Disconnected from %s: %s.\n",
		conn->winname, firetalk_strerror(error));
	conn->online = -1;
	bclearall(conn, 0);

	if (error == FE_RECONNECTING)
		echof(conn, NULL, "Please wait...\n");
	else if ((error != FE_USERDISCONNECT) && getvar_int(conn, "autoreconnect")) {
		echof(conn, NULL, "Attempting to reconnect...\n");
		ua_connect(conn, 0, NULL);
	} else
		echof(conn, NULL, "Type <font color=\"#00FF00\">/%s:connect</font> to reconnect.\n", conn->winname);

	return(HOOK_CONTINUE);
}

static int fireio_userinfo(void *userdata, conn_t *conn, const char *SN, const unsigned char *info, long warning, long online, long idle, long class) {
	if (awayc > 0) {
		int	i;

		assert(awayar != NULL);

		for (i = 0; i < awayc; i++)
			if (firetalk_compare_nicks(conn->conn, SN, awayar[i].name) == FE_SUCCESS) {
				if (awayar[i].gotaway == 0) {
					buddywin_t	*bwin = bgetwin(conn, SN, BUDDY);

					if (bwin == NULL)
						status_echof(conn, "<font color=\"#00FFFF\">%s</font> is now away.\n",
							SN);
					else
						window_echof(bwin, "<font color=\"#00FFFF\">%s</font> is now away.\n",
							user_name(NULL, 0, conn, bwin->e.buddy));
				}
				free(awayar[i].name);
				memmove(awayar+i, awayar+i+1, (awayc-i-1)*sizeof(*awayar));
				awayc--;
				awayar = realloc(awayar, awayc*sizeof(*awayar));
				return(HOOK_CONTINUE);;
			}
	}

	echof(conn, NULL, "Information about %s:\n", SN);

	if (class & (FF_SUBSTANDARD|FF_NORMAL|FF_ADMIN))
	  echof(conn, NULL,
		"</B>&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; <B>Class</B>:%s%s%s",
			(class&FF_SUBSTANDARD)?" AIM":"",
			(class&FF_NORMAL)?" AOLamer":"",
			(class&FF_ADMIN)?" Operator":"");
	{
		buddylist_t *blist = rgetlist(conn, SN);

		if ((blist != NULL) && (blist->caps != NULL))
		  echof(conn, NULL,
			"</B><B>Client features</B>: %s", blist->caps);
	}
	if (warning > 0)
	  echof(conn, NULL,
		"</B>&nbsp; <B>Warning level</B>: %i", warning);
	if (online > 0)
	  echof(conn, NULL,
		"</B>&nbsp; &nbsp; <B>Online time</B>: <B>%s</B>",
		dtime(now-online));
	if (idle > 0)
	  echof(conn, NULL,
		"</B>&nbsp; &nbsp; &nbsp; <B>Idle time</B>: <B>%s</B>",
		dtime(60*idle));
	if (info != NULL)
	  echof(conn, NULL,
		"</B>&nbsp; &nbsp; &nbsp; &nbsp; <B>Profile</B>:<br> %s<br> <hr>", info);
	bupdate();

	return(HOOK_CONTINUE);
}


buddywin_t *cgetwin(conn_t *conn, const char *roomname) {
	buddywin_t *bwin;

	if (*roomname != ':')
		roomname = firetalk_chat_normalize(conn->conn, roomname);
	if ((bwin = bgetwin(conn, roomname, CHAT)) == NULL) {
		bnewwin(conn, roomname, CHAT);
		bwin = bgetwin(conn, roomname, CHAT);
		assert(bwin != NULL);
		bwin->keepafterso = 1;
		bupdate();
	} else
		assert(bwin->keepafterso == 1);
	return(bwin);
}

static int fireio_chat_joined(void *userdata, conn_t *conn, const char *room) {
	buddywin_t *bwin;

	bwin = cgetwin(conn, room);
	bwin->e.chat->offline = 0;
	if (!getvar_int(conn, "autonames"))
		window_echof(bwin, "You are now participating in the %s discussion.\n", room);
	else {
		const char *args[1] = { bwin->winname };

		window_echof(bwin, "You are now participating in the %s discussion. Checking for current participants...\n", room);
		ua_names(conn, 1, args);
	}
	bupdate();

	return(HOOK_CONTINUE);
}

static int fireio_chat_left(void *userdata, conn_t *conn, const char *room) {
	buddywin_t *bwin;

	if ((bwin = bgetwin(conn, room, CHAT)) != NULL) {
		bwin->e.chat->offline = 1;
		firetalk_chat_join(conn->conn, room);
	}
	bupdate();

	return(HOOK_CONTINUE);
}

static int fireio_chat_kicked(void *userdata, conn_t *conn, const char *room, const char *by, const char *reason) {
	buddywin_t *bwin;
	const char *q;

	q = (strchr(room, ' ') != NULL)?"\"":"";

	bwin = cgetwin(conn, room);
	bwin->e.chat->offline = 1;
	bwin->e.chat->isoper = 0;
	if (getvar_int(conn, "chatter") & CH_ATTACKED)
		bwin->waiting = 1;
	window_echof(bwin, "You have been kicked from chat %s%s%s by <font color=\"#00FFFF\">%s</font> (</B><body>%s</body><B>).\n",
			q, room, q, by, reason);
	firetalk_chat_join(conn->conn, room);
	bupdate();

	return(HOOK_CONTINUE);
}

static int fireio_chat_invited(void *userdata, conn_t *conn, const char *room, const char *who, const char *message) {
	const char *q;

	q = (strchr(room, ' ') != NULL)?"\"":"";

	echof(conn, NULL, "<font color=\"#00FFFF\">%s</font> invites you to chat %s%s%s: </B><body>%s</body><B>.\n", who, q, room, q, message);

	return(HOOK_CONTINUE);
}

static int fireio_chat_user_joined(void *userdata, conn_t *conn, const char *room, const char *who, const char *extra) {
	buddywin_t *bwin;
	const char *q;

	q = (strchr(room, ' ') != NULL)?"\"":"";

	bwin = cgetwin(conn, room);

	if (getvar_int(conn, "chatverbose") & CH_USERS)
		bwin->waiting = 1;

	if (extra == NULL)
		window_echof(bwin, "<font color=\"#00FFFF\">%s</font> has joined chat %s%s%s.\n",
			who, q, room, q);
	else
		window_echof(bwin, "<font color=\"#00FFFF\">%s</font> (%s) has joined chat %s%s%s.\n",
			who, extra, q, room, q);
	bupdate();

	return(HOOK_CONTINUE);
}

static int fireio_chat_user_left(void *userdata, conn_t *conn, const char *room, const char *who, const char *reason) {
	buddywin_t *bwin;
	const char *q;

	q = (strchr(room, ' ') != NULL)?"\"":"";

	bwin = cgetwin(conn, room);
	if (getvar_int(conn, "chatverbose") & CH_USERS)
		bwin->waiting = 1;
	if (reason == NULL)
		window_echof(bwin, "<font color=\"#00FFFF\">%s</font> has left chat %s%s%s.\n",
			who, q, room, q);
	else
		window_echof(bwin, "<font color=\"#00FFFF\">%s</font> has left chat %s%s%s (</B><body>%s</body><B>).\n",
			who, q, room, q, (*reason != 0)?reason:"quit");
	bupdate();

	return(HOOK_CONTINUE);
}

static int fireio_chat_user_kicked(void *userdata, conn_t *conn, const char *room, const char *who, const char *by, const char *reason) {
	buddywin_t *bwin;
	const char *q;

	q = (strchr(room, ' ') != NULL)?"\"":"";

	bwin = cgetwin(conn, room);
	if (getvar_int(conn, "chatter") & CH_ATTACKS)
		bwin->waiting = 1;
	window_echof(bwin, "<font color=\"#00FFFF\">%s</font> has been kicked off chat %s%s%s by <font color=\"#00FFFF\">%s</font> (</B><body>%s</body><B>).\n",
		who, q, room, q, by, reason);
	bupdate();

	return(HOOK_CONTINUE);
}

static int fireio_chat_keychanged(void *userdata, conn_t *conn, const char *room, const char *what, const char *by) {
	buddywin_t *bwin;

	bwin = cgetwin(conn, room);
	if (getvar_int(conn, "chatverbose") & CH_MISC)
		bwin->waiting = 1;

	if (what != NULL) {
		if (bwin->e.chat->key != NULL)
			window_echof(bwin, "<font color=\"#00FFFF\">%s</font> has changed the channel key from %s to %s.\n",
				by, bwin->e.chat->key, what);
		else
			window_echof(bwin, "<font color=\"#00FFFF\">%s</font> has changed the channel key to %s.\n",
				by, what);
		STRREPLACE(bwin->e.chat->key, what);
	} else if (bwin->e.chat->key != NULL) {
		window_echof(bwin, "<font color=\"#00FFFF\">%s</font> has cleared the channel key (was %s).\n",
			by, bwin->e.chat->key);
		FREESTR(bwin->e.chat->key);
	}
	bupdate();

	return(HOOK_CONTINUE);
}

#ifdef RAWIRCMODES
static int fireio_chat_modechanged(void *userdata, conn_t *conn, const char *room, const char *mode, const char *by) {
	buddywin_t *bwin;

	bwin = cgetwin(conn, room);
	if (getvar_int(conn, "chatverbose") & CH_MISC)
		bwin->waiting = 1;

	if (mode != NULL)
		window_echof(bwin, "<font color=\"#00FFFF\">%s</font> has set mode <font color=\"#FF00FF\">%s</font>.\n",
			by, mode);
	bupdate();

	return(HOOK_CONTINUE);
}
#endif

static int fireio_chat_oped(void *userdata, conn_t *conn, const char *room, const char *by) {
	buddywin_t *bwin;

	bwin = cgetwin(conn, room);
	if (getvar_int(conn, "chatter") & CH_ATTACKED)
		bwin->waiting = 1;
	bwin->e.chat->isoper = 1;
	bupdate();

	return(HOOK_CONTINUE);
}

static int fireio_chat_user_oped(void *userdata, conn_t *conn, const char *room, const char *who, const char *by) {
	buddywin_t *bwin;

	bwin = cgetwin(conn, room);
	if (getvar_int(conn, "chatverbose") & CH_MISC)
		bwin->waiting = 1;
#ifndef RAWIRCMODES
	if (by != NULL)
		window_echof(bwin, "<font color=\"#00FFFF\">%s</font> has been oped by <font color=\"#00FFFF\">%s</font>.\n",
			who, by);
#endif
	bupdate();

	return(HOOK_CONTINUE);
}

static int fireio_chat_deoped(void *userdata, conn_t *conn, const char *room, const char *by) {
	buddywin_t *bwin;

	bwin = cgetwin(conn, room);
	if (getvar_int(conn, "chatter") & CH_ATTACKED)
		bwin->waiting = 1;
	bwin->e.chat->isoper = 0;
	bupdate();

	return(HOOK_CONTINUE);
}

static int fireio_chat_user_deoped(void *userdata, conn_t *conn, const char *room, const char *who, const char *by) {
	buddywin_t *bwin;

	bwin = cgetwin(conn, room);
	if (getvar_int(conn, "chatter") & CH_ATTACKS)
		bwin->waiting = 1;
	if (firetalk_compare_nicks(conn->conn, conn->sn, who) == FE_SUCCESS)
		bwin->e.chat->isoper = 0;
#ifndef RAWIRCMODES
	window_echof(bwin, "<font color=\"#00FFFF\">%s</font> has been deoped by <font color=\"#00FFFF\">%s</font>.\n",
		who, by);
#endif
	bupdate();

	return(HOOK_CONTINUE);
}

static int fireio_chat_topicchanged(void *userdata, conn_t *conn, const char *room, const char *topic, const char *by) {
	buddywin_t *bwin;
	const char *q;

	q = (strchr(room, ' ') != NULL)?"\"":"";

	bwin = cgetwin(conn, room);
	if (getvar_int(conn, "chatter") & CH_MISC)
		bwin->waiting = 1;
	STRREPLACE(bwin->blurb, topic);
	if (by != NULL)
		window_echof(bwin, "<font color=\"#00FFFF\">%s</font> has changed the topic on %s%s%s to </B><body>%s</body><B>.\n",
			by, q, room, q, topic);
	else
		window_echof(bwin, "Topic for %s: </B><body>%s</body><B>.\n",
			room, topic);
	bupdate();

	return(HOOK_CONTINUE);
}

static int fireio_chat_user_nickchanged(void *userdata, conn_t *conn, const char *room, const char *oldnick, const char *newnick) {
	buddywin_t *bwin;

	bwin = cgetwin(conn, room);
	if (getvar_int(conn, "chatter") & CH_MISC)
		bwin->waiting = 1;
	window_echof(bwin, "<font color=\"#00FFFF\">%s</font> is now known as <font color=\"#00FFFF\">%s</font>.\n",
		oldnick, newnick);
	bupdate();

	return(HOOK_CONTINUE);
}

transfer_t *fnewtransfer(struct firetalk_transfer_t *handle, buddywin_t *bwin, const char *filename,
		const char *from, long size) {
	transfer_t *transfer;

	transfer = calloc(1, sizeof(*transfer));
	assert(transfer != NULL);
	transfer->handle = handle;
	transfer->bwin = bwin;
	transfer->size = size;
	STRREPLACE(transfer->from, from);
	STRREPLACE(transfer->remote, filename);
	transfer->lastupdate = 0;

	return(transfer);
}

void	fremove(transfer_t *transfer) {
	FREESTR(transfer->from);
	FREESTR(transfer->remote);
	FREESTR(transfer->local);
	free(transfer);
}

static int fireio_file_offer(void *userdata, conn_t *conn, struct firetalk_transfer_t *handle, const char *from, const char *filename, long size) {
	if (bgetwin(conn, filename, TRANSFER) == NULL) {
		buddywin_t *bwin;

		bnewwin(conn, filename, TRANSFER);
		bwin = bgetwin(conn, filename, TRANSFER);
		assert(bwin != NULL);
		bwin->waiting = 1;
		bwin->e.transfer = fnewtransfer(handle, bwin, filename, from, size);
		echof(conn, NULL, "File transfer request from <font color=\"#00FFFF\">%s</font> (%s, %lu B).\n",
			from, filename, size);
		window_echof(bwin, "File transfer request from <font color=\"#00FFFF\">%s</font> (%s, %lu B).\n",
			from, filename, size);
		window_echof(bwin, "Type <font color=\"#00FF00\">/accept filename</font> to begin the transfer.\n");
	} else {
		firetalk_file_cancel(conn->conn, handle);
		echof(conn, NULL, "Ignoring duplicate file transfer request from <font color=\"#00FFFF\">%s</font> (%s, %lu B).\n",
			from, filename, size);
	}

	return(HOOK_CONTINUE);
}

static int fireio_file_start(void *userdata, conn_t *conn, struct firetalk_transfer_t *handle, transfer_t *transfer) {
	buddywin_t *bwin;

	bwin = transfer->bwin;
	window_echof(bwin, "Transfer of %s has begun.\n", bwin->e.transfer->remote);
	bwin->e.transfer->started = nowf-0.1;

	return(HOOK_CONTINUE);
}

static int fireio_file_progress(void *userdata, conn_t *conn, struct firetalk_transfer_t *handle, transfer_t *transfer, long bytes, long size) {
	buddywin_t *bwin;

	bwin = transfer->bwin;
	assert(bwin->et == TRANSFER);
	if (bwin->e.transfer->handle == NULL)
		bwin->e.transfer->handle = handle;
	assert(handle == bwin->e.transfer->handle);

	if (size != bwin->e.transfer->size) {
		window_echof(bwin, "File size for %s changed from %lu to %lu.\n",
			bwin->e.transfer->remote, bwin->e.transfer->size, size);
		bwin->e.transfer->size = size;
	}
	bwin->e.transfer->bytes = bytes;

	if ((bwin->e.transfer->lastupdate+5) < now) {
		window_echof(bwin, "STATUS %s/s, %lu/%lu (%i%%)\n",
			dsize(bytes/(nowf-bwin->e.transfer->started)),
			bytes, size, (int)(100.0*bytes/size));
		bwin->e.transfer->lastupdate = now;
	}

	return(HOOK_CONTINUE);
}

static int fireio_file_finish(void *userdata, conn_t *conn, struct firetalk_transfer_t *handle, transfer_t *transfer, long size) {
	buddywin_t *bwin;

	bwin = transfer->bwin;
	assert(handle == bwin->e.transfer->handle);
	if (size != bwin->e.transfer->size) {
		window_echof(bwin, "File size for %s changed from %lu to %lu.\n",
			bwin->e.transfer->remote, bwin->e.transfer->size, size);
		bwin->e.transfer->size = size;
	}
	window_echof(bwin, "STATUS %s/s, %lu/%lu (%i%%)\n",
		dsize(size/(nowf-bwin->e.transfer->started)),
		size, size, 100);
	echof(conn, NULL, "File transfer (%s -> %s) completed.\n",
		bwin->e.transfer->remote, bwin->e.transfer->local);
	bwin->waiting = 1;
	bwin->e.transfer->bytes = bwin->e.transfer->size;
	bwin->e.transfer->size *= -1;

	return(HOOK_CONTINUE);
}

static int fireio_file_error(void *userdata, conn_t *conn, struct firetalk_transfer_t *handle, transfer_t *transfer, int error) {
	buddywin_t *bwin;

	bwin = transfer->bwin;
	assert(handle == bwin->e.transfer->handle);
	echof(conn, NULL, "Error receiving %s: %s.\n",
		bwin->e.transfer->remote, firetalk_strerror(error));
	bwin->waiting = 1;

	return(HOOK_CONTINUE);
}

void	naim_lastupdate(conn_t *conn) {
	int	autohide = script_getvar_int("autohide");

	if ((conn->lastupdate + autohide) < nowf)
		conn->lastupdate = nowf;
	else if ((conn->lastupdate + SLIDETIME) < nowf)
		conn->lastupdate = nowf - SLIDETIME - SLIDETIME/autohide;
}

void	fireio_hook_init(void) {
	void	*mod = NULL;

	HOOK_ADD(postselect,		mod, fireio_postselect,		100, NULL);
	HOOK_ADD(proto_doinit,		mod, fireio_doinit,		100, NULL);
	HOOK_ADD(proto_connected,	mod, fireio_connected,		100, NULL);
	HOOK_ADD(proto_connectfailed,	mod, fireio_connectfailed,	100, NULL);
	HOOK_ADD(proto_newnick,		mod, fireio_proto_newnick,	100, NULL);
	HOOK_ADD(proto_nickchanged,	mod, fireio_proto_nickchanged,	100, NULL);
	HOOK_ADD(proto_warned,		mod, fireio_warned,		100, NULL);
	HOOK_ADD(proto_error_msg,	mod, fireio_error_msg,		100, NULL);
	HOOK_ADD(proto_error_disconnect, mod, fireio_error_disconnect,	100, NULL);
	HOOK_ADD(proto_userinfo,	mod, fireio_userinfo,		100, NULL);
	HOOK_ADD(proto_buddyadded,	mod, fireio_buddyadded,		100, NULL);
	HOOK_ADD(proto_buddyremoved,	mod, fireio_buddyremoved,	100, NULL);
	HOOK_ADD(proto_buddy_coming,	mod, fireio_buddy_coming,	100, NULL);
	HOOK_ADD(proto_buddy_going,	mod, fireio_buddy_going,	100, NULL);
	HOOK_ADD(proto_buddy_away,	mod, fireio_buddy_away,		100, NULL);
	HOOK_ADD(proto_buddy_unaway,	mod, fireio_buddy_unaway,	100, NULL);
	HOOK_ADD(proto_buddy_idle,	mod, fireio_buddy_idle,		100, NULL);
	HOOK_ADD(proto_buddy_eviled,	mod, fireio_buddy_eviled,	100, NULL);
	HOOK_ADD(proto_buddy_capschanged, mod, fireio_buddy_capschanged, 100, NULL);
	HOOK_ADD(proto_buddy_typing,	mod, fireio_buddy_typing,	100, NULL);
	HOOK_ADD(proto_denyadded,	mod, fireio_denyadded,		100, NULL);
	HOOK_ADD(proto_denyremoved,	mod, fireio_denyremoved,	100, NULL);
	HOOK_ADD(proto_recvfrom,	mod, fireio_recvfrom_ignorelist, 10, NULL);
	HOOK_ADD(proto_recvfrom,	mod, fireio_recvfrom_decrypt,	20, NULL);
	HOOK_ADD(proto_recvfrom,	mod, fireio_recvfrom_log,	50, NULL);
	HOOK_ADD(proto_recvfrom,	mod, fireio_recvfrom_beep,	50, NULL);
	HOOK_ADD(proto_recvfrom,	mod, fireio_recvfrom_autobuddy,	50, NULL);
	HOOK_ADD(proto_recvfrom,	mod, fireio_recvfrom_display_user, 100, NULL);
	HOOK_ADD(proto_recvfrom,	mod, fireio_recvfrom_display_chat, 150, NULL);
	HOOK_ADD(proto_chat_joined,	mod, fireio_chat_joined,	100, NULL);
	HOOK_ADD(proto_chat_left,	mod, fireio_chat_left,		100, NULL);
	HOOK_ADD(proto_chat_oped,	mod, fireio_chat_oped,		100, NULL);
	HOOK_ADD(proto_chat_deoped,	mod, fireio_chat_deoped,	100, NULL);
	HOOK_ADD(proto_chat_kicked,	mod, fireio_chat_kicked,	100, NULL);
	HOOK_ADD(proto_chat_invited,	mod, fireio_chat_invited,	100, NULL);
	HOOK_ADD(proto_chat_user_joined, mod, fireio_chat_user_joined,	100, NULL);
	HOOK_ADD(proto_chat_user_left,	mod, fireio_chat_user_left,	100, NULL);
	HOOK_ADD(proto_chat_user_oped,	mod, fireio_chat_user_oped,	100, NULL);
	HOOK_ADD(proto_chat_user_deoped, mod, fireio_chat_user_deoped,	100, NULL);
	HOOK_ADD(proto_chat_user_kicked, mod, fireio_chat_user_kicked,	100, NULL);
	HOOK_ADD(proto_chat_user_nickchanged, mod, fireio_chat_user_nickchanged, 100, NULL);
	HOOK_ADD(proto_chat_topicchanged, mod, fireio_chat_topicchanged, 100, NULL);
	HOOK_ADD(proto_chat_keychanged,	mod, fireio_chat_keychanged,	100, NULL);
	HOOK_ADD(proto_file_offer,	mod, fireio_file_offer,		100, NULL);
	HOOK_ADD(proto_file_start,	mod, fireio_file_start,		100, NULL);
	HOOK_ADD(proto_file_progress,	mod, fireio_file_progress,	100, NULL);
	HOOK_ADD(proto_file_finish,	mod, fireio_file_finish,	100, NULL);
	HOOK_ADD(proto_file_error,	mod, fireio_file_error,		100, NULL);
}
