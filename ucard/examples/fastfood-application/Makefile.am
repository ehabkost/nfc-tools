# $Id: Makefile.am 583 2011-01-01 16:09:54Z romain $

AM_CFLAGS = @LIBFREEFARE_CFLAGS@
INCLUDES = -I $(top_srcdir)/libucard -I $(top_srcdir)/common

noinst_PROGRAMS = fastfood-application

fastfood_application_SOURCES = application.c
nodist_fastfood_application_SOURCES = fastfood.c fastfood.h
fastfood_application_LDADD = $(top_builddir)/libucard/libucard.la \
			     $(top_builddir)/common/libucard-convenience.la

application.o: fastfood.h

fastfood.c fastfood.h: fastfood.xml $(top_srcdir)/stylesheets/ucard.xsl $(top_srcdir)/stylesheets/common.xsl
	@$(XSLTPROC) $(top_srcdir)/stylesheets/ucard.xsl $(top_srcdir)/examples/fastfood-application/fastfood.xml

lint: check-xml-syntax

.PHONY: check-xml-syntax

check-xml-syntax:
	@$(XMLLINT) --noout --schema $(top_srcdir)/stylesheets/ucard.xsd $(top_srcdir)/examples/fastfood-application/fastfood.xml

CLEANFILES = fastfood.c fastfood.h

EXTRA_DIST = fastfood.xml
