# $Id$

AM_CFLAGS = @LIBFREEFARE_CFLAGS@
INCLUDES = -I $(top_srcdir)/libucard -I $(top_srcdir)/common

bin_PROGRAMS = acces-application

acces_application_SOURCES = application.c
nodist_acces_application_SOURCES = acces.c acces.h ../access_file.c ../access_file.h $(top_srcdir)/demo/gpio.c $(top_srcdir)/demo/gpio.h
acces_application_LDADD = $(top_builddir)/libucard/libucard.la \
			  $(top_builddir)/common/libucard-convenience.la

application.o: acces.h 

acces.c acces.h: acces.xml $(top_srcdir)/stylesheets/ucard.xsl $(top_srcdir)/stylesheets/common.xsl
	@$(XSLTPROC) $(top_srcdir)/stylesheets/ucard.xsl $(top_srcdir)/demo/acces/acces-application/acces.xml

CLEANFILES = acces.c acces.h

EXTRA_DIST = acces.xml