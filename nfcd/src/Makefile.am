INCLUDES = $(all_includes)
METASOURCES = AUTO
noinst_HEADERS = NfcdObject.h

BUILT_SOURCES = NfcdObjectGlue.h

bin_PROGRAMS = nfcd

nfcd_SOURCES = nfcd.c
AM_LDFLAGS = @LIBNFC_LIBS@ @DBUS_LIBS@
AM_CFLAGS = @LIBNFC_CFLAGS@ @DBUS_CFLAGS@ -D_XOPEN_SOURCE=500 -Wall -pedantic -ggdb

CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = NfcdObject.xml

NfcdObjectGlue.h: NfcdObject.xml
	dbus-binding-tool --prefix=nfcd_object --mode=glib-server $< > $@

