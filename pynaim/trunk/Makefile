CC = gcc
CPPFLAGS = -Wall -I/usr/include/python2.5 -I../naim-current/libfiretalk -I../naim-current/include
LIBS = -lpython2.5
OBJ = conn.lo hooks.lo pd.lo pynaim.lo

pynaim.la: ${OBJ}
	libtool --mode=link ${CC} -o pynaim.la -rpath /usr/local/lib/naim -module -avoid-version ${OBJ} ${LIBS}

pynaim.lo: pynaim.c _default_py.h

%.lo: %.c
	libtool --mode=compile ${CC} ${CPPFLAGS} -c -o $@ $<

_default_py.h: default.py
	sed -e 's/\\/\\\\/g' -e 's/"/\\"/g' \
	    < default.py \
	    | awk 'BEGIN {printf("static const char *default_py =\n");} /^.*$$/ {printf("\t\"%s\\n\"\n", $$0);} END {printf(";\n");}' \
	    > _default_py.h
