# $Id: Makefile.am 580 2010-12-29 23:04:24Z romain $

AM_CFLAGS = @LIBFREEFARE_CFLAGS@
INCLUDES = -I $(top_srcdir)/libucard

bin_PROGRAMS = ucard-recover-owner

ucard_recover_owner_SOURCES = ucard-recover-owner.c
ucard_recover_owner_LDADD = $(top_builddir)/libucard/libucard.la
