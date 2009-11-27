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

#define PAM_NFC_FILE "/etc/pam_nfc.conf"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <syslog.h>
#include <stdarg.h>
#include <pwd.h>

#include <string.h>
#include <sys/perm.h>

#include <crypt.h>

#include "nfc-access.h"

/*
 * here, we make a definition for the externally accessible function
 * in this file (this definition is required for static a module
 * but strongly encouraged generally) it is used to instruct the
 * modules include file to define the function prototypes.
 */

#define PAM_SM_AUTH

#include <security/pam_modules.h>
#include <security/_pam_macros.h>

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
	struct stat conffile_fileinfo;

	FILE *conffile;
	char confline[256];

	const char *user=NULL;

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
	}

	if ( stat ( PAM_NFC_FILE, &conffile_fileinfo ) )
	{
		_pam_log ( LOG_NOTICE, "Couldn't open " PAM_NFC_FILE );
		return PAM_SERVICE_ERR;
	}

	if ( ( conffile_fileinfo.st_mode & S_IWOTH )
	        || !S_ISREG ( conffile_fileinfo.st_mode ) )
	{
		/* If the file is world writable or is not a normal file, return error */
		_pam_log ( LOG_ERR, PAM_NFC_FILE
		           " is either world writable or not a normal file" );
		return PAM_AUTH_ERR;
	}

	conffile = fopen ( PAM_NFC_FILE, "r" );
	if ( conffile == NULL )   /* Check that we opened it successfully */
	{
		_pam_log ( LOG_ERR,
		           "Error opening " PAM_NFC_FILE );
		return PAM_SERVICE_ERR;
	}

	/* DO NFC STUFF HERE */
	char uid[128];
	char output[256];

	if(0 == nfc_get_uid(uid)) {
		sprintf(output, "%s %s", user, crypt(uid,"RC"));
		_pam_log ( LOG_ERR, "NFC uid fetched: %s", uid );
	} else {
		_pam_log ( LOG_ERR,
		           "Unable to read NFC card." );
		return PAM_AUTH_ERR;
	}

	/* There should be no more errors from here */
	retval=PAM_AUTH_ERR;
	/* This loop assumes that PAM_SUCCESS == 0
	   and PAM_AUTH_ERR != 0 */
	while ( ( fgets ( confline,sizeof ( confline )-1, conffile ) != NULL )
	        && retval )
	{
		if ( confline[strlen ( confline ) - 1] == '\n' )
			confline[strlen ( confline ) - 1] = '\0';

		/* compare uid with confline in conffile */
		if ( ( strcmp ( output, confline ) ) ==0 ) retval=PAM_SUCCESS;
	}
	fclose ( conffile ); /* close file */

	if ( retval == PAM_SUCCESS )
		_pam_log ( LOG_DEBUG, "access allowed for '%s'", user );
	return retval;
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
