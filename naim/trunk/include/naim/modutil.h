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
	char	*name;
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

#define HOOK_CALL(x, ...)					\
	do {							\
		static chain_t *_chain = NULL;			\
		int	i;					\
								\
		if (_chain == NULL)				\
			_chain = hook_findchain(#x);		\
								\
		for (i = 0; i < _chain->count; i++) {		\
			int	ret;				\
								\
			_chain->hooks[i].passes++;		\
			if (_chain->hooks[i].userdata != NULL)	\
				ret = _chain->hooks[i].func(_chain->hooks[i].userdata, ##__VA_ARGS__); \
			else					\
				ret = _chain->hooks[i].func(__VA_ARGS__); \
			if (ret != HOOK_CONTINUE) {		\
				_chain->hooks[i].hits++;	\
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
chain_t	*hook_findchain(const char *name);
void	hook_add(const char *name, void *mod, mod_hook_t func, void *userdata, int weight, const char *hookname);
void	hook_del(const char *name, void *mod, mod_hook_t func, void *userdata);

int	mod_fd_register(int fd, int type, char *buf, int buflen,
		void (*func)());
void	mod_fd_unregister(int i);

void	mod_fd_read_raw(int fd, void (*func)(int, int));
void	mod_fd_read_buf(int fd, void (*func)(char *, int), char *, int);

void	mod_fd_connect_raw(const char *host, int port, void (*func)(int));
void	mod_fd_write_buf(int fd, const char *buf, int buflen);

#endif /* naim_h */
