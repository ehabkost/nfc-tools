# $Id$

AM_CFLAGS = @LIBFREEFARE_CFLAGS@
INCLUDES = -I $(top_srcdir)/libucard -I $(top_srcdir)/common

bin_PROGRAMS = transport-info

transport_info_SOURCES = info.c
transport_info_LDADD = $(top_builddir)/libucard/libucard.la \
		       $(top_builddir)/common/libucard-convenience.la