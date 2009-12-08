/* pam_nfc module
 * Copyright (C) 2009 Romuald Conty <romuald.conty@free.fr>
 *
 * Many thanks to Denis Bodor <lefinnois@lefinnois.net>
 *   Author of "crypt2g" which been used as template for this pam module.
 *
 * And many thanks to Roel Verdult <roel@libnfc.org>
 *   Author of "libnfc" which is the library this module based on.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#include <sys/stat.h>
#include <sys/types.h>

#include <pwd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#if defined(HAVE_SYS_PERM_H)
#include <sys/perm.h>
#endif /* HAVE_SYS_PERM_H */

#if defined(HAVE_CRYPT_H)
#include <crypt.h>
#endif /* HAVE_CRYPT_H */

#include "nfcauth.h"

/*
 * here, we make a definition for the externally accessible function
 * in this file (this definition is required for static a module
 * but strongly encouraged generally) it is used to instruct the
 * modules include file to define the function prototypes.
 */

#define PAM_SM_AUTH

#if defined(HAVE_SECURITY_PAM_MODULES_H)
#include <security/pam_modules.h>
#endif /* HAVE_SECURITY_PAM_MODULES_H */

#if defined(HAVE_SECURITY_PAM__MACROS_H)
#include <security/_pam_macros.h>
#endif /* HAVE_SECURITY_PAM__MACROS_H */

#if defined(HAVE_SECURITY_OPENPAM_H)
#  include <security/openpam.h>
#endif /* HAVE_SECURITY_OPENPAM_H */

#if defined(HAVE_SECURITY_PAM_APPL_H)
#  include <security/pam_appl.h>
#endif /* HAVE_SECURITY_PAM_APPL_H */

/* some syslogging */
static void _pam_log ( int err, const char *format, ... )
{
	va_list args;
	va_start ( args, format );
	openlog ( "pam_nfc", LOG_CONS|LOG_PID, LOG_AUTH );
	vsyslog ( err, format, args );
	va_end ( args );
	closelog();
}

/* --- authentication management functions (only) --- */

PAM_EXTERN
int pam_sm_authenticate ( pam_handle_t *pamh,int flags,int argc
                          ,const char **argv )
{
	int retval = PAM_AUTH_ERR;

	char confline[256];

	char *user = NULL;

	retval = pam_get_user ( pamh, &user, NULL );
	if ( retval != PAM_SUCCESS )
	{
		_pam_log ( LOG_ERR, "get user returned error: %s",
		           pam_strerror ( pamh,retval ) );
		return retval;
	}
	if ( user == NULL || *user == '\0' )
	{
		_pam_log ( LOG_ERR, "username not known" );
		return retval;
	}

	if (!(nfcauth_check ())) return PAM_SERVICE_ERR;

	return (nfcauth_authorize (user)) ? PAM_SUCCESS : PAM_AUTH_ERR;
}

PAM_EXTERN
int pam_sm_setcred ( pam_handle_t *pamh,int flags,int argc
                     ,const char **argv )
{
	return PAM_SUCCESS;
}


#ifdef PAM_STATIC

/* static module data */

struct pam_module _pam_nfc_modstruct =
{
	"pam_nfc",
	pam_sm_authenticate,
	pam_sm_setcred,
	NULL,
	NULL,
	NULL,
	NULL,
};

#endif

/* end of module definition */
