EXTRA_DIST = firetalk.m4

noinst_LIBRARIES = libfiretalk.a
libfiretalk_a_SOURCES = \
	firetalk.c \
	firetalk-int.h \
	irc.c \
	pdutil.c \
	toc2.c \
	toc2_uuids.h
	
#	lily.c \
# foo

check_PROGRAMS = tester
tester_SOURCES = \
	tester.c
tester_LDADD = \
	libfiretalk.a

TESTS = ${check_PROGRAMS}

include_HEADERS = firetalk.h
