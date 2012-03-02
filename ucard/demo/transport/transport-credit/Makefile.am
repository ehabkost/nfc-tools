# $Id$

AM_CFLAGS = @LIBFREEFARE_CFLAGS@
INCLUDES = -I $(top_srcdir)/libucard -I $(top_srcdir)/common

bin_PROGRAMS = transport-credit

transport_credit_SOURCES = credit.c
transport_credit_LDADD = $(top_builddir)/libucard/libucard.la \
			 $(top_builddir)/common/libucard-convenience.la