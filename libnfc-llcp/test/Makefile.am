# $Id$

INCLUDES = $(CUTTER_CFLAGS) -I$(top_srcdir)/libnfc-llcp
LIBS = $(CUTTER_LIBS)

if WITH_CUTTER

TESTS = run-test.sh
TESTS_ENVIRONMENT = NO_MAKE=yes CUTTER="$(CUTTER)"

cutter_unit_test_libs = \
			test_llc_connection.la \
			test_llc_link.la \
			test_llcp_pdu.la \
			test_llcp_parameters.la \
			test_llc_service.la \
			test_dummy_mac_link.la \
			test_mac_link.la

if WITH_DEBUG
noinst_LTLIBRARIES = $(cutter_unit_test_libs)
else
check_LTLIBRARIES = $(cutter_unit_test_libs)
endif

AM_LDFLAGS = -module -rpath $(libdir) -avoid-version -no-undefined

test_llc_connection_la_SOURCES = test_llc_connection.c
test_llc_connection_la_LIBADD = $(top_builddir)/libnfc-llcp/libnfc-llcp.la
test_llc_connection_la_CFLAGS = $(LIBNFC_CFLAGS)

test_llc_link_la_SOURCES = test_llc_link.c
test_llc_link_la_LIBADD = $(top_builddir)/libnfc-llcp/libnfc-llcp.la

test_llcp_pdu_la_SOURCES = test_llcp_pdu.c
test_llcp_pdu_la_LIBADD = $(top_builddir)/libnfc-llcp/libnfc-llcp.la

test_llcp_parameters_la_SOURCES = test_llcp_parameters.c
test_llcp_parameters_la_LIBADD = $(top_builddir)/libnfc-llcp/libnfc-llcp.la

test_llc_service_la_SOURCES = test_llc_service.c
test_llc_service_la_LIBADD = $(top_builddir)/libnfc-llcp/libnfc-llcp.la

test_dummy_mac_link_la_SOURCES = test_dummy_mac_link.c
test_dummy_mac_link_la_LIBADD = $(top_builddir)/libnfc-llcp/libnfc-llcp.la
test_dummy_mac_link_la_CFLAGS = $(LIBNFC_CFLAGS)

test_mac_link_la_SOURCES = test_mac_link.c
test_mac_link_la_LIBADD = $(top_builddir)/libnfc-llcp/libnfc-llcp.la
test_mac_link_la_CFLAGS = $(LIBNFC_CFLAGS)
test_mac_link_la_LIBS = $(LIBNFC_LIBS)

echo-cutter:
	@echo $(CUTTER)

EXTRA_DIST = run-test.sh
CLEANFILES = *.gcno

endif
