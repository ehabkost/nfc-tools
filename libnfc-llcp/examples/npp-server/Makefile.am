# $Id$

INCLUDES = -I$(top_srcdir)/libnfc-llcp
AM_CFLAGS = $(LIBNFC_CFLAGS)

noinst_PROGRAMS = npp-server

npp_server_SOURCES = npp-server.c

npp_server_LDADD = $(LIBNFC_LIBS) $(top_builddir)/libnfc-llcp/libnfc-llcp.la
