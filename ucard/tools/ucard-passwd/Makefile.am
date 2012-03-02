# $Id$

AM_CFLAGS = @LIBFREEFARE_CFLAGS@
INCLUDES = -I $(top_srcdir)/libucard

bin_PROGRAMS = ucard-passwd

ucard_passwd_SOURCES = ucard-passwd.c
ucard_passwd_LDADD = $(top_builddir)/libucard/libucard.la
