sysconfdir=@sysconfdir@

SUBDIRS = debug nfcconf modules

INCLUDES = $(all_includes)

AM_CFLAGS = -DSYSCONFDIR=\"${sysconfdir}\" @LIBNFC_CFLAGS@
AM_LDFLAGS = @LIBNFC_LIBS@

bin_PROGRAMS = nfc-eventd
nfc_eventd_SOURCES = nfc-eventd.c
nfc_eventd_LDADD = $(top_builddir)/src/debug/libdebug.la \
	$(top_builddir)/src/nfcconf/libnfcconf.la @LIBNFCCONF@
noinst_HEADERS = types.h
