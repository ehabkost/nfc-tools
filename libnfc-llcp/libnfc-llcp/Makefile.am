# $Id$

AM_CFLAGS = @LIBNFC_CFLAGS@
AM_LDFLAGS = @LIBNFC_LIBS@

lib_LTLIBRARIES = libnfc-llcp.la

libnfc_llcp_la_SOURCES = \
			 llcp_pdu.c \
			 llcp_parameters.c

EXTRA_DIST = llcp.h llcp_pdu.h llcp_parameters.h
CLEANFILES = *.gcno
