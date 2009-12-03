/*
 * Entry adder for pam_nfc
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
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(HAVE_CRYPT_H)
#include <crypt.h>
#endif /* HAVE_CRYPT_H */

#include "nfc-access.h"

int main ( int argc, char *argv[] )
{
	char *user;
	char nfc_uid[128];
	char output[256];

	if ( argc<2 )
	{
		printf ( "Usage : nfc-pam-add [USER] >> ${sysconfdir}/pam_nfc.conf\n" );
		exit ( 0 );
	}
	if ( ( strcmp ( argv[1], "--help" ) ) ==0 )
	{
		printf (
		    "nfc-pam-add - Entry adder for pam_nfc module\n"
		    "Copyright(c)2009 Romuald Conty - Redistribute under the terme of GNU GPL"
		    "\n\n"
		    "    Usage : nfc-pam-add [USER] >> ${sysconfdir}/pam_nfc.conf\n" );
		exit ( 0 );
	}

	if ( ( strcmp ( argv[1], "--version" ) ) ==0 )
	{
		printf (
		    "nfc-pam-add - Entry adder for pam_nfc module\n"
		    "Copyright(c)2009 Romuald Conty - Redistribute under the terme of GNU GPL"
		    "\n"
		    "Version 0.1 (what a ugly code, isn't it ?)\n" );
		exit ( 0 );
	}

	user=argv[1]; /* first argument should be a username */

	/* DO NFC STUFF HERE */
	if ( 0 == nfc_get_uid(nfc_uid) ) {
	// 	sprintf(output, "%s %s", user, nfc_uid);
		sprintf(output, "%s %s", user, crypt(nfc_uid,"RC"));
		printf("%s\n", output);
	} else {
		exit ( 1 );
	}
	return ( 0 );
}
