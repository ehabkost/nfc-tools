AC_INIT(pam_nfc.c)

AM_INIT_AUTOMAKE(pam_nfc, 0.0)

AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AM_PROG_LIBTOOL

AC_CHECK_LIB(dl, main)

#----------------------------------------------------------------------
# libnfc Cheks
AC_CHECK_HEADER(nfc/nfc.h, [], have_nfc=no)
if test "$have_nfc" = "no"; then
	AC_MSG_ERROR("The libnfc headers are missing")
fi
AC_CHECK_LIB(nfc, nfc_connect, [], have_nfc=no)
if test "$have_nfc" = "no"; then
	AC_MSG_ERROR("The libnfc library is missing")
fi

#----------------------------------------------------------------------
# PAM Checks
AC_CHECK_HEADER(security/pam_modules.h,
		AC_DEFINE(HAVE_SECURITY_PAM_MODULES_H),
		have_security_pam_modules=no)

AC_CHECK_HEADER(security/_pam_macros.h,
		AC_DEFINE(HAVE_SECURITY__PAM_MACROS_H),
		have_security__pam_macros=no)

AC_CHECK_HEADER(security/pam_appl.h,
		AC_DEFINE(HAVE_SECURITY_PAM_APPL_H),
		have_security_pam_appl=no)

AC_CHECK_HEADER(security/openpam.h,
		AC_DEFINE(HAVE_SECURITY_OPENPAM_H),
		have_security_openpam=no,
		[pam_types.h])

if test "$have_security_pam_modules" = "no" -a \
	"$have_security__pam_macros" = "no" -a \
	"$have_security_pam_appl" = "no" -a \
	"$have_security_openpam" = "no"; then
	AC_MSG_ERROR("The PAM headers are missing")
fi
old_LIBS=$LIBS
AC_CHECK_LIB(pam, pam_start, have_pam=yes, have_pam=no)
if test "$have_pam" = "no"; then
	AC_MSG_ERROR("The PAM library is missing")
fi
LIBS=$old_LIBS

#----------------------------------------------------------------------
# PAM Module Directory
AC_ARG_WITH([pam-dir], [AC_HELP_STRING([--with-pam-dir=DIR],
	    [directory to install pam module in])], [],
	    [with_pam_dir='${prefix}/lib/security'])
PAM_DEST_DIR="$with_pam_dir"
AC_SUBST(PAM_DEST_DIR)

#----------------------------------------------------------------------
# libcrypt Checks
AC_CHECK_HEADERS(crypt.h, AC_DEFINE(HAVE_CRYPT_H))
AC_CHECK_LIB(crypt, crypt)

AC_CHECK_HEADERS(sys/perm.h, AC_DEFINE(HAVE_SYS_PERM_H))

#----------------------------------------------------------------------
# Backend
AC_ARG_WITH([backend], [AC_HELP_STRING([--with-backend=BACKEND],
	    [Backend to use for authorization storage])], [],
	    [with_backend='config'])
case "$with_backend" in
config)
	AC_DEFINE(PAM_NFC_BACKEND_CONFIG)
	;;
*)
	AC_ERROR(Unknown backend '$with_backend'.)
	;;
esac

SHAVE_INIT([], [enable])
AC_CONFIG_FILES([shave shave-libtool])
AC_OUTPUT(Makefile)
