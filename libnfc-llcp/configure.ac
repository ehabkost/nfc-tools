# $Id$

AC_INIT([libnfc-llcp], [0.1.0])

AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CC
AC_PROG_LIBTOOL

AC_C_INLINE

AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE

AM_SILENT_RULES([yes])

# Debug support (default:no)
AC_ARG_ENABLE([debug],AS_HELP_STRING([--enable-debug],[Enable debug output]),[enable_debug=$enableval],[enable_debug="no"])
AM_CONDITIONAL(WITH_DEBUG, [test x"$enable_debug" = xyes])
if test x"$enable_debug" != x"yes"; then
    CFLAGS="$CFLAGS -DNDEBUG"
fi

AC_DEFINE([_XOPEN_SOURCE], [600], [Define to 500 if Single Unix conformance is wanted, 600 for sixth revision.])
AC_DEFINE([_BSD_SOURCE], [1], [Define on BSD to activate all library features])
AC_DEFINE([__BSD_VISIBLE], [1], [Define on BSD to activate all library features])
AC_DEFINE([_GNU_SOURCE], [1], [Define on GNU/Linux to activate all library features])

AC_FUNC_MALLOC
AC_TYPE_INT8_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT8_T

AC_CHECK_FUNCS([strcasecmp])
AC_CHECK_FUNCS([strdup])

AC_CHECK_HEADERS([fcntl.h])
AC_CHECK_HEADERS([sys/param.h])
AC_CHECK_HEADERS([pthread_np.h])

AC_CHECK_DECLS([pthread_set_name_np(pthread_t, const char *)], [], [], [[#include <pthread_np.h>]])

CFLAGS="$CFLAGS -std=c99"

# Checks for log4c
AC_PATH_PROG([LOG4C_CONFIG], [log4c-config])
if test x"$LOG4C_CONFIG" != x""; then
    log4c_CFLAGS=`$LOG4C_CONFIG --cflags`
    log4c_LIBS=`$LOG4C_CONFIG --libs`
    AC_SUBST([log4c_CFLAGS])
    AC_SUBST([log4c_LIBS])
    AC_DEFINE([HAS_LOG4C], [1], [Define to 1 if log4c is available.])
fi
AM_CONDITIONAL(HAS_LOG4C, [test x"$LOG4C_CONFIG" != x""])

# Checks for graphviz
AC_PATH_PROG([DOT], [dot])
AC_SUBST(DOT)
AM_CONDITIONAL(HAS_GRAPHVIZ, [test x"$DOT" != x""])

# Checks for mscgen
AC_PATH_PROG([MSCGEN], [mscgen])
AC_SUBST(MSCGEN)
AM_CONDITIONAL(HAS_MSCGEN, [test x"$MSCGEN" != x""])

# Checks for pkg-config modules.
LIBNFC_REQUIRED_VERSION="1.4.0"
PKG_CHECK_MODULES([LIBNFC], [libnfc >= $LIBNFC_REQUIRED_VERSION], [], [AC_MSG_ERROR([libnfc >= $LIBNFC_REQUIRED_VERSION is mandatory.])])

PKG_CONFIG_REQUIRES="libnfc"
AC_SUBST([PKG_CONFIG_REQUIRES])

# Checks for cutter unit tests framework
CUTTER_REQUIRED_VERSION=1.1.2
m4_ifdef([AC_CHECK_CUTTER], [AC_CHECK_CUTTER([>= $CUTTER_REQUIRED_VERSION])], [ac_cv_use_cutter="no"])
if test x$ac_cv_with_cutter = xyes -a x$ac_cv_use_cutter = xno; then
    AC_MSG_ERROR([cutter >= $CUTTER_REQUIRED_VERSION is mandatory.])
fi
AM_CONDITIONAL([WITH_CUTTER], [test "$ac_cv_use_cutter" != "no"])

m4_ifdef([AC_CHECK_COVERAGE], [AC_CHECK_COVERAGE])

if test x$cutter_enable_coverage = xyes; then
    CFLAGS="$CFLAGS -O0 -fprofile-arcs -ftest-coverage"
fi

AC_OUTPUT([
	   Makefile
	   doc/Makefile
	   libnfc-llcp/Makefile
	   test/Makefile
	   tools/Makefile
	   tools/llcp-pdu-explain/Makefile
	   tools/llcp-test-server/Makefile
	   ])
