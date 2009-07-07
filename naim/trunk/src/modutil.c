/*  _ __   __ _ ___ __  __
** | '_ \ / _` |_ _|  \/  | naim
** | | | | (_| || || |\/| | Copyright 1998-2009 Daniel Reed <n@ml.org>
** |_| |_|\__,_|___|_|  |_| ncurses-based chat client
*/
#include "naim-int.h"
#include <naim/modutil.h>
#include <ltdl.h>

static inline chain_t *_hook_findchain(const char *name) {
	extern lt_dlhandle dl_self;
	chain_t *hook;
	char	buf[100];

	snprintf(buf, sizeof(buf), "chain_%s", name);

	hook = lt_dlsym(dl_self, buf);
	return(hook);
}

void	hook_add(const char *name, void *mod, mod_hook_t func, void *userdata, int weight, const char *funcname) {
	chain_t	*chain = _hook_findchain(name);
	int	pos;

	for (pos = 0; (pos < chain->count) && (chain->hooks[pos].weight <= weight); pos++)
		;

	chain->hooks = realloc(chain->hooks, (chain->count+1) * sizeof(*(chain->hooks)));
	memmove(chain->hooks+pos+1, chain->hooks+pos, (chain->count-pos) * sizeof(*(chain->hooks)));
	chain->hooks[pos].weight = weight;
	chain->hooks[pos].passes = 0;
	chain->hooks[pos].hits = 0;
	chain->hooks[pos].func = func;
	chain->hooks[pos].userdata = userdata;
	chain->hooks[pos].name = strdup(funcname);
	chain->hooks[pos].mod = mod;
	chain->count++;
}

void	hook_del(const char *name, void *mod, mod_hook_t func, void *userdata) {
	chain_t	*chain = _hook_findchain(name);
	int	i;

	for (i = 0; (i < chain->count)
		&& ((chain->hooks[i].mod != mod)
		 || (chain->hooks[i].func != func)
		 || (chain->hooks[i].userdata != userdata)); i++)
		;
	if (i < chain->count) {
		free(chain->hooks[i].name);
		memmove(chain->hooks+i, chain->hooks+i+1, (chain->count-i-1) * sizeof(*(chain->hooks)));
		chain->hooks = realloc(chain->hooks, (chain->count-1) * sizeof(*(chain->hooks)));
		chain->count--;
	}
}

mod_fd_list_t	*mod_fd_listar = NULL;
int	mod_fd_listc = 0;

int	mod_fd_register(int fd, int type, char *buf, int buflen,
		void (*func)()) {
	int	i = mod_fd_listc;

	if (type == (O_RDONLY+1))
		for (i = 0; i < mod_fd_listc; i++) {
			if ((mod_fd_listar[i].fd == fd)
				&& (mod_fd_listar[i].type == type))
				break;
		}
	if (i == mod_fd_listc) {
		mod_fd_listc++;
		mod_fd_listar = realloc(mod_fd_listar,
			sizeof(*mod_fd_listar)*mod_fd_listc);
		mod_fd_listar[i].fd = fd;
	}
	mod_fd_listar[i].type = type;
	mod_fd_listar[i].buf = buf;
	mod_fd_listar[i].buflen = buflen;
	mod_fd_listar[i].func = func;
	return(i);
}

void	mod_fd_read_raw(int fd, void (*func)(int, int)) {
	mod_fd_register(fd, (O_RDONLY+1), NULL, 0, func);
}

void	mod_fd_read_buf(int fd, void (*func)(char *, int), char *buf,
		int buflen) {
	mod_fd_register(fd, (O_RDONLY+1), buf, buflen, func);
}

#if 0
void	mod_fd_connect_raw(const char *host, int port, void (*func)(int)) {
	int	fd = firetalk_internal_connect_host(host, port);

	mod_fd_register(fd, MOD_FD_CONNECT, NULL, 0, func);
}
#endif

void	mod_fd_write_buf(int fd, const char *buf, int buflen) {
	mod_fd_register(fd, MOD_FD_WRITE, (char *)buf, buflen, NULL);
}

void	mod_fd_unregister(int i) {
	assert(i < mod_fd_listc);
	memmove(mod_fd_listar+i, mod_fd_listar+i+1,
		sizeof(*mod_fd_listar)*(mod_fd_listc-i-1));
	mod_fd_listc--;
	mod_fd_listar = realloc(mod_fd_listar,
		sizeof(*mod_fd_listar)*mod_fd_listc);
}
