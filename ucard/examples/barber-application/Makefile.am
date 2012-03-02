# $Id$

AM_CFLAGS = @LIBFREEFARE_CFLAGS@
INCLUDES = -I $(top_srcdir)/libucard -I $(top_srcdir)/common

noinst_PROGRAMS = barber-application

barber_application_SOURCES = application.c
barber_application_LDADD = $(top_builddir)/libucard/libucard.la \
			   $(top_builddir)/common/libucard-convenience.la
