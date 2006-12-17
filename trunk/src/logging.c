/*  _ __   __ _ ___ __  __
** | '_ \ / _` |_ _|  \/  | naim
** | | | | (_| || || |\/| | Copyright 1998-2006 Daniel Reed <n@ml.org>
** |_| |_|\__,_|___|_|  |_| ncurses-based chat client
*/
#include <naim/naim.h>
#include <utime.h>

#include "naim-int.h"

extern faimconf_t faimconf;
extern time_t	now;
extern const char *home;

extern int colormode G_GNUC_INTERNAL;
int	colormode = COLOR_HONOR_USER;

static const unsigned char *naim_normalize(const unsigned char *const name) {
	static char newname[2048];
	int	i, j = 0;

	for (i = 0; (name[i] != 0) && (j < sizeof(newname)-1); i++)
		if ((name[i] == '/') || (name[i] == '.'))
			newname[j++] = '_';
		else if (name[i] != ' ')
			newname[j++] = tolower(name[i]);
	newname[j] = 0;
	return(newname);
}

static int touch(const char *filename, time_t atime, time_t mtime) {
	struct utimbuf buf = {
		actime: atime,
		modtime: mtime,
	};

	return(utime(filename, &buf));
}

static int makedir(const char *d) {
	char	*dir;

	if (*d != '/') {
		static char buf[1024];

		snprintf(buf, sizeof(buf), "%s/%s", home, d);
		d = buf;
	}
	dir = strdup(d);
	while (chdir(d) != 0) {
		strcpy(dir, d);
		while (chdir(dir) != 0) {
			char	*pdir = strrchr(dir, '/');

			if (mkdir(dir, 0700) != 0)
				if (errno != ENOENT) {
					chdir(home);
					free(dir);
					return(-1);
				}
			if (pdir == NULL)
				break;
			*pdir = 0;
		}
	}
	chdir(home);
	free(dir);
	return(0);
}

static void playback_fix(const char *logdir) {
	struct stat sb;
	char	nhtml[256];

	snprintf(nhtml, sizeof(nhtml), "%s.html", logdir);

	if ((stat(logdir, &sb) == 0) && S_ISREG(sb.st_mode)) {
		if ((stat(nhtml, &sb) == 0) && S_ISREG(sb.st_mode))
			remove(logdir);
		else
			rename(logdir, nhtml);
	}

	if (stat(logdir, &sb) != 0)
		makedir(logdir);
	else
		assert(S_ISDIR(sb.st_mode));

	if ((stat(nhtml, &sb) == 0) && S_ISREG(sb.st_mode)) {
		char	buf[256];
		struct tm *tm;

		tm = gmtime(&(sb.st_mtime));
		snprintf(buf, sizeof(buf), "%s/", logdir);
		strftime(buf+strlen(buf), sizeof(buf)-strlen(buf), "%Y%m%d%H%M-0.html", tm);
		rename(nhtml, buf);
	}
}

void	playback_file_estimator(conn_t *const conn, buddywin_t *const bwin, struct h_t *h, FILE *rfile, const int lines) {
	char	buf[2048];
	int	maxlen = lines*faimconf.wstatus.widthx;
	long	filesize, playbackstart, playbacklen, pos;
	time_t	lastprogress = now;

	fseek(rfile, 0, SEEK_END);
	while (((filesize = ftell(rfile)) == -1) && (errno == EINTR))
			;
	assert(filesize >= 0);
	if (filesize > maxlen) {
		fseek(rfile, -maxlen, SEEK_CUR);
		while ((fgetc(rfile) != '\n') && !feof(rfile))
			;
	} else
		fseek(rfile, 0, SEEK_SET);
	while (((playbackstart = ftell(rfile)) == -1) && (errno == EINTR))
		;
	assert(playbackstart >= 0);
	pos = 0;
	playbacklen = filesize-playbackstart;
	while (fgets(buf, sizeof(buf), rfile) != NULL) {
		long	len = strlen(buf);

		pos += len;
		if (buf[len-1] == '\n') {
			hhprint(h, buf, len-1);
			hendblock(h);
		} else
			hhprint(h, buf, len);

		if ((now = time(NULL)) > lastprogress) {
			nw_statusbarf("Redrawing window for %s (%li lines left).",
				bwin->winname, lines*(playbacklen-pos)/playbacklen);
			lastprogress = now;
		}
	}
}

void	playback_file(conn_t *const conn, buddywin_t *const bwin, struct h_t *h, FILE *rfile, const int lines) {
	char	buf[2048];
	long	filesize, pos;
	time_t	lastprogress = now;

	fseek(rfile, 0, SEEK_END);
	while (((filesize = ftell(rfile)) == -1) && (errno == EINTR))
			;
	assert(filesize >= 0);
	fseek(rfile, 0, SEEK_SET);
	pos = 0;
	while (fgets(buf, sizeof(buf), rfile) != NULL) {
		long	len = strlen(buf);

		pos += len;
		if (buf[len-1] == '\n') {
			hhprint(h, buf, len-1);
			hendblock(h);
		} else
			hhprint(h, buf, len);

		if ((now = time(NULL)) > lastprogress) {
			nw_statusbarf("Redrawing window for %s (%li lines left).",
				bwin->winname, lines*(filesize-pos)/filesize);
			lastprogress = now;
		}
	}
}

void	playback(conn_t *const conn, buddywin_t *const bwin, int lines) {
	struct h_t *h = hhandle(&(bwin->nwin));
	const char *logdir;

	nw_statusbarf("Redrawing window for %s.", bwin->winname);

	assert(bwin->nwin.logfile != NULL);
	fflush(bwin->nwin.logfile);
	bwin->nwin.dirty = 0;

	script_setvar("conn", conn->winname);
	script_setvar("cur", naim_normalize(bwin->winname));

	logdir = script_expand(script_getvar("logdir"));

	playback_fix(logdir);

	if (script_getvar_int("color"))
		colormode = COLOR_FORCE_ON;
	else
		colormode = COLOR_FORCE_OFF;

	while (lines > 0) {
		const char *rfilename;
		FILE	*rfile;
		int	linesset, linestmp = bwin->nwin.logfilelines;

		if (((rfilename = playback_ffind(logdir, lines, &linesset)) != NULL) && (rfile = fopen(rfilename, "r"))) {
			if (linesset == 0)
				playback_file_estimator(conn, bwin, h, rfile, lines);
			else {
				playback_file(conn, bwin, h, rfile, linesset);
				lines -= linesset;
			}
			fclose(rfile);
		} else
			break;
	}

	colormode = COLOR_HONOR_USER;
}
