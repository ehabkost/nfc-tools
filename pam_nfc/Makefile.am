
noinst_LTLIBRARIES = libnfc-access.la
libnfc_access_la_SOURCES = \
	nfc-access.c

pamlibdir = $(PAM_DEST_DIR)
pamlib_LTLIBRARIES = pam_nfc.la

pam_nfc_la_SOURCES = \
	pam_nfc.c

pam_nfc_la_LDFLAGS = \
	-module -avoid-version

pam_nfc_la_LIBADD = \
	libnfc-access.la
	-lpam

bin_PROGRAMS = nfc-pam-add

nfc_pam_add_SOURCES = \
	nfc-pam-add.c

nfc_pam_add_LDADD = \
	libnfc-access.la

EXTRA_DIST = nfc-access.h
