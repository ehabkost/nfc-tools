# $Id$

AM_CFLAGS = @LIBFREEFARE_CFLAGS@
INCLUDES = -I $(top_srcdir)/libucard -I $(top_srcdir)/common

bin_PROGRAMS = acces-soft 

acces_soft_SOURCES = acces.c ../access_file.c ../access_file.h
acces_soft_LDADD = $(top_builddir)/libucard/libucard.la @CRYPT_LIB@ \
		   $(top_builddir)/common/libucard-convenience.la
