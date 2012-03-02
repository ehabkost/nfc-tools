/*-
 * Copyright (C) 2010, Romain Tartiere.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 * $Id: ucard-keygen.c 505 2010-11-15 18:14:47Z romain $
 */

#include <err.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <openssl/err.h>
#include <openssl/rand.h>

void
usage (char *prog)
{
    fprintf (stderr, "usage: %s [-1 | -3] [-c] [-n count] [-v version]\n", prog);
}

int
main (int argc, char *argv[])
{
    char ch;
    int key_3des = 1;
    int copt = 0;
    int v, n = 1;
    uint8_t key_version = 0;

    char *prog = argv[0];

    while ((ch = getopt (argc, argv, "13cn:v:")) != -1) {
	switch (ch) {
	case '1':
	    key_3des = 0;
	    break;
	case '3':
	    key_3des = 1;
	    break;
	case 'c':
	    copt = 1;
	    break;
	case 'n':
	    if (1 != sscanf (optarg, "%d", &n)) {
		fprintf (stderr, "Invalid number '%s'.\n", optarg);
		usage (prog);
		exit (EXIT_FAILURE);
	    }
	    break;
	case 'v':
	    if (1 != sscanf (optarg, "%d", &v)) {
		fprintf (stderr, "Invalid key version '%s'.\n", optarg);
		usage (prog);
		exit (EXIT_FAILURE);
	    }
	    key_version = (uint8_t) v;
	    break;
	default:
	    usage (prog);
	    exit (EXIT_FAILURE);
	}
    }
    argc -= optind;
    argv += optind;

    while (n--) {

    /* Generate key */
    uint8_t key_data[16];
    if (!RAND_bytes (key_data, sizeof (key_data))) {
	errx (EXIT_FAILURE, "Can't generate strong cryptographic data: %s", ERR_error_string (ERR_get_error (), NULL));
    }
    if (!key_3des) {
	memcpy (key_data + 8, key_data, 8);
    }

    /* Set version */
    for (int i = 0; i < 8; i++) {
	if ((key_version >> (7-i)) & 0x01) {
	    key_data[i] |= 0x01;
	    if (key_3des)
		key_data[i+8] &= 0xfe;
	    else
		key_data[i+8] |= 0x01;
	} else {
	    key_data[i] &= 0xfe;
	    if (key_3des)
		key_data[i+8] |= 0x01;
	    else
		key_data[i+8] &= 0xfe;
	}
    }

    /* Display generated key */
    int key_size = (key_3des) ? 16 : 8 ;
    if (copt) {
	fprintf (stdout, "{ ");
	for (int i = 0; i < key_size - 1; i++) {
	    fprintf (stdout, "0x%02x, ", key_data[i]);
	}
	fprintf (stdout, "0x%02x }\n", key_data[key_size - 1]);
    } else {
	fprintf (stdout, "0x");
	for (int i = 0; i < key_size; i++) {
	    fprintf (stdout, "%02x", key_data[i]);
	}
	fprintf (stdout, "\n");
    }

    }

    exit(EXIT_SUCCESS);
}
