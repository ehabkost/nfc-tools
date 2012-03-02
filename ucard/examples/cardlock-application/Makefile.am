# $Id: Makefile.am 583 2011-01-01 16:09:54Z romain $

AM_CFLAGS = @LIBFREEFARE_CFLAGS@
INCLUDES = -I $(top_srcdir)/libucard -I $(top_srcdir)/common

noinst_PROGRAMS = cardlock-application

cardlock_application_SOURCES = application.c
nodist_cardlock_application_SOURCES = cardlock.c cardlock.h
cardlock_application_LDADD = $(top_builddir)/libucard/libucard.la \
			     $(top_builddir)/common/libucard-convenience.la

application.o: cardlock.h

cardlock.c cardlock.h: cardlock.xml $(top_srcdir)/stylesheets/ucard.xsl $(top_srcdir)/stylesheets/common.xsl
	@$(XSLTPROC) $(top_srcdir)/stylesheets/ucard.xsl $(top_srcdir)/examples/cardlock-application/cardlock.xml

lint: check-xml-syntax

.PHONY: check-xml-syntax

check-xml-syntax:
	@$(XMLLINT) --noout --schema $(top_srcdir)/stylesheets/ucard.xsd $(top_srcdir)/examples/cardlock-application/cardlock.xml

CLEANFILES = cardlock.c cardlock.h

EXTRA_DIST = cardlock.xml
