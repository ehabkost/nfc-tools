INCLUDES = $(all_includes)
METASOURCES = AUTO
moduledir=$(pkglibdir)/modules
nem_execute_la_LDFLAGS = -module -no-undefined
noinst_HEADERS = nem_common.h nem_execute.h
module_LTLIBRARIES = nem_execute.la
nem_execute_la_SOURCES = nem_execute.c
nem_execute_la_DEPENDENCIES = -lnfc
nem_execute_la_LIBADD = $(top_builddir)/src/debug/libdebug.la \
	$(top_builddir)/src/nfcconf/libnfcconf.la

