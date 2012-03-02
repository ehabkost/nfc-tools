# $Id$

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = libucard stylesheets common tools examples demo

SVN_KEYWORDS_FILES_LIST_CMD = find $(top_srcdir) -name '*.[hc]' -a ! -name config.h \
			      -o -name Makefile.am
svn-keywords:
	@echo Update files svn:keywords...
	@$(SVN_KEYWORDS_FILES_LIST_CMD) | xargs svn propset -q svn:keywords Id
	$(MAKE) svn-keywords-check

svn-keywords-check:
	@echo "Files missing a '$$"Id"$$' vcs keyword:"
	@$(SVN_KEYWORDS_FILES_LIST_CMD) | xargs grep -L '\$$Id[^$$]*\$$'

