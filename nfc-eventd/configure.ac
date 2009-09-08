AC_INIT(nfc-eventd, 0.0.1, rconty@il4p.fr)

AC_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE

AC_LANG_C
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AC_PROG_RANLIB

AC_PATH_PROG(PKG_CONFIG, pkg-config)

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_STDBOOL
AC_CHECK_HEADERS([stdio.h stdlib.h stdint.h stddef.h stdbool.h usb.h])

# Checks for types
AC_TYPE_SIZE_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_INT32_T

# --enable-debug support (default:no)
AC_ARG_ENABLE([debug],AS_HELP_STRING([--enable-debug],[Debug flags]),[enable_debug=$enableval],[enable_debug="no"])
 
AC_MSG_CHECKING(for debug flag)
AC_MSG_RESULT($enable_debug)
 
if test x"$enable_debug" = "xyes"
then
  CFLAGS="$CFLAGS -g3 -Wall -DDEBUG -pedantic"
fi
AC_SUBST([DEBUG_CFLAGS])

PKG_CHECK_MODULES(LIBNFC, libnfc, [WITH_NFC=1], [WITH_NFC=0])

if test x"$WITH_NFC" = "x0"; then
  AC_MSG_ERROR([libnfc is mandatory.])
fi
AC_SUBST(LIBNFC_LIBS)
AC_SUBST(LIBNFC_CFLAGS)

AC_SUBST(LIBNFCCONF)
LIBNFCCONF="\${top_builddir}/src/nfcconf/libnfcconf.la"

# Set C standard to C99
CFLAGS="$CFLAGS -std=c99"

AC_CONFIG_FILES([
		Makefile
		src/Makefile
		src/nfcconf/Makefile
		src/debug/Makefile
		src/modules/Makefile
		])

AC_OUTPUT
