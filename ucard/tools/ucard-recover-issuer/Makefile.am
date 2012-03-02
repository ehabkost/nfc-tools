# $Id$

AM_CFLAGS = @LIBFREEFARE_CFLAGS@
INCLUDES = -I $(top_srcdir)/libucard

bin_PROGRAMS = ucard-recover-issuer

ucard_recover_issuer_SOURCES = ucard-recover-issuer.c
ucard_recover_issuer_LDADD = $(top_builddir)/libucard/libucard.la
