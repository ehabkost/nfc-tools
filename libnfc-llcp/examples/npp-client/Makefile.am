# $Id$

INCLUDES = -I$(top_srcdir)/libnfc-llcp
AM_CFLAGS = $(LIBNFC_CFLAGS)

noinst_PROGRAMS = npp-client

npp_client_SOURCES = npp-client.c

npp_client_LDADD = $(LIBNFC_LIBS) $(top_builddir)/libnfc-llcp/libnfc-llcp.la
