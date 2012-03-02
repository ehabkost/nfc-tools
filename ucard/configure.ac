AC_INIT([libucard], [0.0.0])

AC_CONFIG_MACRO_DIR([m4])

# Use automake
AM_INIT_AUTOMAKE
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_LN_S

AC_PATH_PROG([XSLTPROC], [xsltproc])
if test x"$XSLTPROC" = x""; then AC_MSG_ERROR([xsltproc not found.]); fi
AC_SUBST(XSLTPROC)

AC_PATH_PROG([XMLLINT], [xmllint], [AC_MSG_ERROR([xmllint not found.])])
if test x"$XMLLINT" = x""; then AC_MSG_ERROR([xmllint not found.]); fi
AC_SUBST(XMLLINT)

# Checks for libraries.
LIBFREEFARE_REQUIRED_VERSION="0.3.3"
AC_CHECK_LIB([crypto], [RAND_bytes], [], [AC_MSG_ERROR([Cannot find libcrypto.])])
PKG_CHECK_MODULES([LIBFREEFARE], [libfreefare >= $LIBFREEFARE_REQUIRED_VERSION], [], [AC_MSG_ERROR([libfreefare >= $LIBFREEFARE_REQUIRED_VERSION is mandatory.])])
LIBS_saved="$LIBS"
AC_CHECK_LIB([crypt], [crypt], [CRYPT_LIB="-lcrypt"], [CRYPT_LIB=""])
LIBS="$LIBS_saved"
AC_SUBST(CRYPT_LIB)

# Checks for header files.
AC_HEADER_STDBOOL
AC_CHECK_HEADERS([sys/time.h], [], [AC_MSG_ERROR([Cannot find sys/time.h.])])
AC_CHECK_HEADERS([openssl/err.h openssl/rand.h], [], [AC_MSG_ERROR([Cannot find openssl headers.])])
AC_CHECK_HEADER([crypt.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_UINT8_T
AC_TYPE_INT32_T
AC_TYPE_UINT32_T
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memset], [], [AC_MSG_ERROR([A memset() function is required.])])
AC_CHECK_FUNCS([select], [], [AC_MSG_ERROR([A select() function is required.])])
AC_CHECK_FUNCS([strchr], [], [AC_MSG_ERROR([A strchr() function is required.])])
AC_CHECK_FUNCS([strdup], [], [AC_MSG_ERROR([A strdup() function is required.])])

# Setup some defaults compilation settings
CFLAGS="$CFLAGS -std=c99"

AC_DEFINE([_XOPEN_SOURCE], [600], [Define to 500 if Single Unix conformance is wanted, 600 for sixth revision.])
AC_DEFINE([_BSD_SOURCE], [1], [Define on BSD to activate all library features])

AC_OUTPUT([Makefile
	   common/Makefile
           demo/Makefile
           demo/acces/Makefile
           demo/acces/acces-application/Makefile
           demo/acces/acces-soft/Makefile
           demo/fidelite/Makefile
           demo/fidelite/fidelite-application/Makefile
           demo/fidelite/fidelite-debit/Makefile
           demo/fidelite/fidelite-info/Makefile
           demo/transport/Makefile
           demo/transport/transport-application/Makefile
           demo/transport/transport-credit/Makefile
           demo/transport/transport-info/Makefile
	   examples/Makefile
	   examples/barber-application/Makefile
	   examples/cardlock-application/Makefile
	   examples/fastfood-application/Makefile
           examples/marchant-application/Makefile
	   examples/passport-application/Makefile
	   libucard/Makefile
	   stylesheets/Makefile
	   tools/Makefile
	   tools/ucard-explain/Makefile
	   tools/ucard-keygen/Makefile
	   tools/ucard-passwd/Makefile
	   tools/ucard-prepare/Makefile
	   tools/ucard-recover-issuer/Makefile
	   tools/ucard-recover-owner/Makefile
	   ])
