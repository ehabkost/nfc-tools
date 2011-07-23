# $Id$

INCLUDES = -I$(top_srcdir)/libnfc-llcp
LIBS = -lrt
AM_CFLAGS = $(LIBNFC_CFLAGS)

noinst_PROGRAMS = llcp-test-server

llcp_test_server_SOURCES = llcp-test-server.c \
			   connected-echo-server.c \
			   connectionless-echo-server.c

llcp_test_server_LDADD = $(LIBNFC_LIBS) $(top_builddir)/libnfc-llcp/libnfc-llcp.la

EXTRA_DIST = connected-echo-server.h \
	     connectionless-echo-server.h
