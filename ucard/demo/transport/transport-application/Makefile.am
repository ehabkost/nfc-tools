# $Id$ 

AM_CFLAGS = @LIBFREEFARE_CFLAGS@
INCLUDES = -I $(top_srcdir)/libucard -I $(top_srcdir)/common

bin_PROGRAMS = transport-application

transport_application_SOURCES = application.c
nodist_transport_application_SOURCES = transport.c transport.h $(top_srcdir)/demo/gpio.c $(top_srcdir)/demo/gpio.h
transport_application_LDADD = $(top_builddir)/libucard/libucard.la \
			      $(top_builddir)/common/libucard-convenience.la

application.o: transport.h

transport.c transport.h: transport.xml $(top_srcdir)/stylesheets/ucard.xsl $(top_srcdir)/stylesheets/common.xsl
	@$(XSLTPROC) $(top_srcdir)/stylesheets/ucard.xsl $(top_srcdir)/demo/transport/transport-application/transport.xml

CLEANFILES = transport.c transport.h

EXTRA_DIST = transport.xml
