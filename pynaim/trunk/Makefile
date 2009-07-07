CC = gcc
CPPFLAGS = -I/usr/include/python2.5 -I../naim-current/libfiretalk -I../naim-current/include
LIBS = -lpython2.5
OBJ = pynaim.lo

pynaim.la: ${OBJ}
	libtool --mode=link gcc -o pynaim.la -rpath /usr/local/lib/naim -module -avoid-version ${OBJ} ${LIBS}

pynaim.lo: pynaim.c _default_py.h
	libtool --mode=compile gcc ${CPPFLAGS} -c -o pynaim.lo pynaim.c

_default_py.h: default.py
	sed -e 's/\\/\\\\/g' -e 's/"/\\"/g' \
	    < default.py \
	    | awk 'BEGIN {printf("static const char *default_py =\n");} /^.*$$/ {printf("\t\"%s\\n\"\n", $$0);} END {printf(";\n");}' \
	    > _default_py.h
