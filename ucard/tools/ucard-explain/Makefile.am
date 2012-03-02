# $Id: Makefile.am 552 2010-12-07 13:09:10Z romain $

AM_CFLAGS = @LIBFREEFARE_CFLAGS@
INCLUDES = -I $(top_srcdir)/libucard

bin_PROGRAMS = ucard-explain

ucard_explain_SOURCES = ucard-explain.c
ucard_explain_LDADD = $(top_builddir)/libucard/libucard.la
