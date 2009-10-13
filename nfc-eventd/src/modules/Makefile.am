INCLUDES = $(all_includes)
METASOURCES = AUTO
moduledir=$(pkglibdir)/modules
noinst_HEADERS = nem_common.h nem_execute.h
module_LTLIBRARIES = nem_execute.la

nem_execute_la_SOURCES = nem_execute.c
nem_execute_la_LDFLAGS = -module -no-undefined @LIBNFC_LIBS@
nem_execute_la_CFLAGS = @LIBNFC_CFLAGS@
nem_execute_la_LIBADD = $(top_builddir)/src/debug/libdebug.la \
	$(top_builddir)/src/nfcconf/libnfcconf.la

if DBUS_ENABLED
BUILT_SOURCES = nfc-dbus-object.h

noinst_HEADERS += nem_dbus.h nfc-dbus-object.h

module_LTLIBRARIES += nem_dbus.la
nem_dbus_la_SOURCES = nem_dbus.c
nem_dbus_la_LDFLAGS = -module -no-undefined @LIBNFC_LIBS@ @DBUS_LIBS@
nem_dbus_la_CFLAGS = @LIBNFC_CFLAGS@ @DBUS_CFLAGS@
nem_dbus_la_LIBADD = $(top_builddir)/src/debug/libdebug.la \
	$(top_builddir)/src/nfcconf/libnfcconf.la

CLEANFILES = $(BUILT_SOURCES)
endif

EXTRA_DIST = nfc-dbus-object.xml

nfc-dbus-object.h: nfc-dbus-object.xml
	dbus-binding-tool --prefix=nfc_object --mode=glib-server  $<> $@

