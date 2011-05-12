# $Id$

AC_INIT([libnfc-llcp], [0.1.0])

AC_CONFIG_MACRO_DIR([m4])

AC_PROG_LIBTOOL

AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE

AM_SILENT_RULES([yes])

# Debug support (default:no)
AC_ARG_ENABLE([debug],AS_HELP_STRING([--enable-debug],[Enable debug output]),[enable_debug=$enableval],[enable_debug="no"])
AM_CONDITIONAL(WITH_DEBUG, [test x"$enable_debug" = xyes])

AC_FUNC_MALLOC
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT8_T

CFLAGS="$CFLAGS -std=c99"

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
	   libnfc-llcp/Makefile
	   test/Makefile
	   ])
