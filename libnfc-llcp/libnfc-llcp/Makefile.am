# $Id$

AM_CFLAGS = @LIBNFC_CFLAGS@
AM_LDFLAGS = @LIBNFC_LIBS@ -lrt

if HAS_LOG4C
AM_CFLAGS += $(log4c_CFLAGS)
AM_LDFLAGS += $(log4c_LIBS)
endif

lib_LTLIBRARIES = libnfc-llcp.la

libnfc_llcp_la_SOURCES = \
			 llcp.c \
			 llcp_pdu.c \
			 llcp_parameters.c \
			 llc_link.c \
			 llc_service.c \
			 llc_service_sdp.c 

if HAS_LOG4C
libnfc_llcp_la_SOURCES += llcp_log.c
endif

EXTRA_DIST = llcp.h llcp_log.h llcp_pdu.h llcp_parameters.h llc_service.h
CLEANFILES = *.gcno
