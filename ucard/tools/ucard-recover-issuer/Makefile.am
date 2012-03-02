# $Id: Makefile.am 580 2010-12-29 23:04:24Z romain $

AM_CFLAGS = @LIBFREEFARE_CFLAGS@
INCLUDES = -I $(top_srcdir)/libucard

bin_PROGRAMS = ucard-recover-issuer

ucard_recover_issuer_SOURCES = ucard-recover-issuer.c
ucard_recover_issuer_LDADD = $(top_builddir)/libucard/libucard.la
