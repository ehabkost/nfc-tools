# $Id$

SUBDIRS = acces \
	transport \
	fidelite

AM_CFLAGS = @LIBFREEFARE_CFLAGS@
INCLUDES = -I $(top_srcdir)/libucard -I $(top_srcdir)/common

bin_PROGRAMS = general-info

general_info_SOURCES = info.c
general_info_LDADD = $(top_builddir)/libucard/libucard.la \
		     $(top_builddir)/common/libucard-convenience.la
