# $Id$

INCLUDES = -I$(top_srcdir)/libnfc-llcp
AM_CFLAGS = $(LIBNFC_CFLAGS)

noinst_PROGRAMS = llcp-test-client

llcp_test_client_SOURCES = llcp_test_client.c

llcp_test_client_LDADD = $(LIBNFC_LIBS) $(top_builddir)/libnfc-llcp/libnfc-llcp.la
