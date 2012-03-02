# $Id: Makefile.am 583 2011-01-01 16:09:54Z romain $

AM_CFLAGS = @LIBFREEFARE_CFLAGS@
INCLUDES = -I $(top_srcdir)/libucard -I $(top_srcdir)/common

noinst_PROGRAMS = marchant-application

marchant_application_SOURCES = application.c
nodist_marchant_application_SOURCES = marchant.c marchant.h
marchant_application_LDADD = $(top_builddir)/libucard/libucard.la \
			     $(top_builddir)/common/libucard-convenience.la

application.o: marchant.h

marchant.c marchant.h: marchant.xml $(top_srcdir)/stylesheets/ucard.xsl $(top_srcdir)/stylesheets/common.xsl
	@$(XSLTPROC) $(top_srcdir)/stylesheets/ucard.xsl $(top_srcdir)/examples/marchant-application/marchant.xml

lint: check-xml-syntax

.PHONY: check-xml-syntax

check-xml-syntax:
	@$(XMLLINT) --noout --schema $(top_srcdir)/stylesheets/ucard.xsd $(top_srcdir)/examples/marchant-application/marchant.xml

CLEANFILES = marchant.c marchant.h

EXTRA_DIST = marchant.xml
