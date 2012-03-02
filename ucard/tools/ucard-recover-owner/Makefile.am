# $Id$

AM_CFLAGS = @LIBFREEFARE_CFLAGS@
INCLUDES = -I $(top_srcdir)/libucard

bin_PROGRAMS = ucard-recover-owner

ucard_recover_owner_SOURCES = ucard-recover-owner.c
ucard_recover_owner_LDADD = $(top_builddir)/libucard/libucard.la
