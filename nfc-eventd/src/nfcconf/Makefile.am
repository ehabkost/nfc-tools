# Process this file with automake to create Makefile.in

MAINTAINERCLEANFILES = Makefile.in

DISTCLEANFILES = lex-parse.c
EXTRA_DIST = README.nfcconf lex-parse.l

noinst_HEADERS = internal.h nfcconf.h
#noinst_PROGRAMS = test-conf
noinst_LTLIBRARIES = libnfcconf.la

libnfcconf_la_SOURCES = nfcconf.h internal.h nfcconf.c parse.c write.c nfclex.c

#test_conf_SOURCES = test-conf.c
#test_conf_LDADD = libnfcconf.la
