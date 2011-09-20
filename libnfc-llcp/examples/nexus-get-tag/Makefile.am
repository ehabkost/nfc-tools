# $Id$

INCLUDES = -I$(top_srcdir)/libnfc-llcp
AM_CFLAGS = $(LIBNFC_CFLAGS)

noinst_PROGRAMS = nexus-get-tag

nexus_get_tag_SOURCES = nexus-get-tag.c

nexus_get_tag_LDADD = $(LIBNFC_LIBS) $(top_builddir)/libnfc-llcp/libnfc-llcp.la
