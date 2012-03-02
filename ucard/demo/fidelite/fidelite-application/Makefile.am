# $Id$
AM_CFLAGS = @LIBFREEFARE_CFLAGS@
INCLUDES = -I $(top_srcdir)/libucard -I $(top_srcdir)/common

bin_PROGRAMS = fidelite-application

fidelite_application_SOURCES = application.c
nodist_fidelite_application_SOURCES = fidelite.c fidelite.h $(top_srcdir)/demo/gpio.c $(top_srcdir)/demo/gpio.h
fidelite_application_LDADD = $(top_builddir)/libucard/libucard.la \
			  $(top_builddir)/common/libucard-convenience.la

application.o: fidelite.h

fidelite.c fidelite.h: fidelite.xml $(top_srcdir)/stylesheets/ucard.xsl $(top_srcdir)/stylesheets/common.xsl
	@$(XSLTPROC) $(top_srcdir)/stylesheets/ucard.xsl $(top_srcdir)/demo/fidelite/fidelite-application/fidelite.xml

CLEANFILES = fidelite.c fidelite.h

EXTRA_DIST = fidelite.xml