/*  _ __   __ _ ___ __  __
** | '_ \ / _` |_ _|  \/  | naim
** | | | | (_| || || |\/| | Copyright 1998-2009 Daniel Reed <n@ml.org>
** |_| |_|\__,_|___|_|  |_| ncurses-based chat client
*/
#ifndef modutil_h
#define modutil_h	1

#include <naim/naim.h>
#include <fcntl.h>
#include <time.h>

typedef int (*mod_hook_t)(void *mod, ...);
typedef struct {
	int	count;
	struct {
		int	weight;
		unsigned long passes, hits;
		mod_hook_t func;
		char	*name;
		void	*mod, *userdata;
	} *hooks;
} chain_t;
#define HOOK_JUMP	1
#define HOOK_STOP	0
#define HOOK_CONTINUE	(-1)

#define HOOK_DECLARE(x)	chain_t chain_ ## x = { 0, NULL }
#define HOOK_EXT_L(x)	extern chain_t chain_ ## x
#define HOOK_CALL(x, ...)					\
	if ((chain_ ## x).count > 0) do { 			\
		int	i, ret;					\
		for (i = 0; i < chain_ ## x.count; i++) {	\
			chain_ ## x.hooks[i].passes++;		\
			if (chain_ ## x.hooks[i].userdata != NULL) \
				ret = chain_ ## x.hooks[i].func(chain_ ## x.hooks[i].userdata, ##__VA_ARGS__); \
			else					\
				ret = chain_ ## x.hooks[i].func(__VA_ARGS__); \
			if (ret != HOOK_CONTINUE) {		\
				chain_ ## x.hooks[i].hits++;	\
				break;				\
			}					\
		}						\
	} while (0)
#define HOOK_ADD(x, m, f, w)	hook_add(#x, m, (mod_hook_t)f, NULL, w, #f)
#define HOOK_DEL(x, m, f)	hook_del(#x, m, (mod_hook_t)f, NULL)

#define MOD_REMAINLOADED	1
#define MOD_FINISHED		0

#define MODULE_LICENSE(x)	const char module_license[] = #x
#define MODULE_AUTHOR(x)	const char module_author[] = #x
#define MODULE_CATEGORY(x)	const char module_category[] = #x
#define MODULE_DESCRIPTION(x)	const char module_description[] = #x
#define MODULE_VERSION(x)	const double module_version = x



typedef struct {
	int	fd, type, buflen;
	char	*buf;
	void	(*func)();
} mod_fd_list_t;

#define MOD_FD_TYPE(x)	((O_WRONLY+1) | (1 << (2+(x))))

#define MOD_FD_CONNECT	MOD_FD_TYPE(0)
#define MOD_FD_WRITE	MOD_FD_TYPE(1)

/* modutil.c */
void	hook_add(const char *name, void *mod, mod_hook_t func, void *userdata, int weight, const char *funcname);
void	hook_del(const char *name, void *mod, mod_hook_t func, void *userdata);

int	mod_fd_register(int fd, int type, char *buf, int buflen,
		void (*func)());
void	mod_fd_unregister(int i);

void	mod_fd_read_raw(int fd, void (*func)(int, int));
void	mod_fd_read_buf(int fd, void (*func)(char *, int), char *, int);

void	mod_fd_connect_raw(const char *host, int port, void (*func)(int));
void	mod_fd_write_buf(int fd, const char *buf, int buflen);

#endif /* naim_h */
