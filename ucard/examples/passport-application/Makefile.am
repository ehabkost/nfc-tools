# $Id: Makefile.am 583 2011-01-01 16:09:54Z romain $

AM_CFLAGS = @LIBFREEFARE_CFLAGS@
INCLUDES = -I $(top_srcdir)/libucard -I $(top_srcdir)/common

noinst_PROGRAMS = passport-application

passport_application_SOURCES = application.c
nodist_passport_application_SOURCES = passport.c passport.h
passport_application_LDADD = $(top_builddir)/libucard/libucard.la \
			     $(top_builddir)/common/libucard-convenience.la

application.o: passport.h

passport.c passport.h: passport.xml $(top_srcdir)/stylesheets/ucard.xsl $(top_srcdir)/stylesheets/common.xsl
	@$(XSLTPROC) $(top_srcdir)/stylesheets/ucard.xsl $(top_srcdir)/examples/passport-application/passport.xml

lint: check-xml-syntax

.PHONY: check-xml-syntax

check-xml-syntax:
	@$(XMLLINT) --noout --schema $(top_srcdir)/stylesheets/ucard.xsd $(top_srcdir)/examples/passport-application/passport.xml

CLEANFILES = passport.c passport.h

EXTRA_DIST = passport.xml
