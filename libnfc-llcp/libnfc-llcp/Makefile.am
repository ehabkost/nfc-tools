# $Id$

AM_CFLAGS = @LIBNFC_CFLAGS@
AM_LDFLAGS = @LIBNFC_LIBS@ -lrt

llcp_HEADERS = \
		llc_connection.h \
		llc_link.h \
		llc_service.h \
		llcp_pdu.h \
		llcp.h \
		mac.h
llcpdir = $(includedir)/nfc

lib_LTLIBRARIES = libnfc-llcp.la

libnfc_llcp_la_SOURCES = \
			 llcp.c \
			 llcp_pdu.c \
			 llcp_parameters.c \
			 llc_connection.c \
			 llc_link.c \
			 llc_service.c \
			 llc_service_llc.c \
			 llc_service_sdp.c \
			 mac_iso18092.c

if WITH_DEBUG
libnfc_llcp_la_SOURCES += llcp_log.c
endif

EXTRA_DIST = \
	     llcp_log.h \
	     llcp_parameters.h \
	     llc_connection.h \
	     llc_service_llc.h \
	     llc_service_sdp.h
CLEANFILES = *.gcno
