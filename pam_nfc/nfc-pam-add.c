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

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "nfcauth.h"

void	 usage (char *progname);

void
usage (char *progname)
{
	fprintf (stderr, "usage: %s username\n", progname);
	exit (EXIT_FAILURE);
}

int
main (int argc, char *argv[])
{
	if (argc != 2)
		usage (argv[0]);

	char **targets;
	int n;
	if (1 != (n = nfcauth_get_targets (&targets))) {
		errx (EXIT_FAILURE, "%d targets detected.", n);
	}

	if (!(nfcauth_add_authorization (argv[1], targets[0]))) {
		err (EXIT_FAILURE, "Error adding authorisation for user");
	}
	
	free (targets[0]);
	free (targets);

	exit (EXIT_SUCCESS);
}
