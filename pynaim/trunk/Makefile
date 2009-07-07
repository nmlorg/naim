CC = gcc
CPPFLAGS = -I/usr/include/python2.5 -I../naim-current/libfiretalk -I../naim-current/include
LIBS = -lpython2.5
OBJ = pynaim.o

pynaim.so: ${OBJ}
	gcc -shared -o pynaim.so ${OBJ} ${LIBS}
