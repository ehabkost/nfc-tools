
#noinst_LTLIBRARIES = libnfc-access.la
#libnfc_access_la_SOURCES = \
#	nfc-access.c

AM_CFLAGS = -DSYSCONFDIR="\"${sysconfdir}\"" @libnfc_CFLAGS@
AM_LDFLAGS = @libnfc_LIBS@
ACLOCAL_AMFLAGS = -I m4

libnfcauthdir = ${libdir}
libnfcauth_LTLIBRARIES = libnfcauth.la
libnfcauth_la_SOURCES = \
	nfcauth.c \
	nfcauth_backend_config.c

pamlibdir = $(PAM_DEST_DIR)
pamlib_LTLIBRARIES = pam_nfc.la

pam_nfc_la_SOURCES = \
	pam_nfc.c

pam_nfc_la_LDFLAGS = \
	-module -avoid-version

pam_nfc_la_LIBADD = \
	-lnfcauth \
	-lpam

bin_PROGRAMS = pam-nfc-add

pam_nfc_add_SOURCES = \
        pam-nfc-add.c
    
pam_nfc_add_LDADD = \
	-lnfcauth

EXTRA_DIST = nfcauth.h
