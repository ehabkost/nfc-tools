# $Id: Makefile.am 582 2010-12-31 00:31:56Z romain $

AM_CFLAGS = @LIBFREEFARE_CFLAGS@
INCLUDES = -I $(top_srcdir)/libucard

bin_PROGRAMS = ucard-passwd

ucard_passwd_SOURCES = ucard-passwd.c
ucard_passwd_LDADD = $(top_builddir)/libucard/libucard.la
