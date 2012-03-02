# $Id$

AM_CFLAGS = @LIBFREEFARE_CFLAGS@
INCLUDES = -I $(top_srcdir)/libucard -I $(top_srcdir)/common

bin_PROGRAMS = fidelite-debit

fidelite_debit_SOURCES = debit.c
fidelite_debit_LDADD = $(top_builddir)/libucard/libucard.la \
			  $(top_builddir)/common/libucard-convenience.la