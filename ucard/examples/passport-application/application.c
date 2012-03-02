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
 * $Id$
 */

#include <err.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <ucard.h>

#include "passport.h"
#include "on_password_requested.h"

static int	 on_card_presented (struct ucard *ucard, struct ucard_application *ucard_application);

int
main (void)
{
    struct kiosk *kiosk = kiosk_new ();
    struct ucard_application *application = passport_application_new (on_password_requested);

    if (kiosk_devices_scan (kiosk)) {
    kiosk_setup (kiosk, application, on_card_presented);
    kiosk_start (kiosk);
    kiosk_wait (kiosk, NULL);
    kiosk_stop (kiosk);
    }
    kiosk_free (kiosk);

    ucard_application_free (application);

    exit (EXIT_SUCCESS);
}


void
trim_cr (char *data)
{
    char *p = data;
    while (*p) {
	if ('\n' == *p) {
	    *p = '\0';
	    break;
	}
	p++;
    }
}

#define EDIT_FIELD(field, label) do { \
    res = passport_##field##_read_data (ucard, ucard_application, 0, 0, buffer); \
    if (res < 0) { \
	fprintf (stderr, "passport_" #field "_read_data() failed\n"); \
	return 0; \
    } \
    printf ("%s [%s]: ", label, buffer); \
    if (fgets (buffer, BUFSIZ, stdin) == NULL){ \
        printf ("fgets failed: %s\n", strerror(errno)); \
    }\
    trim_cr (buffer); \
    if (buffer[0]) { \
	res = passport_##field##_write_data (ucard, ucard_application, 0, strlen (buffer) + 1, buffer); \
	if (res < 0) { \
	    fprintf (stderr, "passport_" #field "_write_data() failed\n"); \
	    return 0; \
	} \
    } \
} while (0)

static int
on_card_presented (struct ucard *ucard, struct ucard_application *ucard_application)
{
    char buffer[BUFSIZ];
    int res;

    EDIT_FIELD (surname, "Surname");
    EDIT_FIELD (given_names, "Given Names");
    EDIT_FIELD (nationnality, "Nationnality");
    EDIT_FIELD (date_of_birth, "Date of Birth");
    EDIT_FIELD (sex, "Sex (M/F)");
    EDIT_FIELD (place_of_birth, "Place of Birth");
    EDIT_FIELD (date_of_issue, "Date of Issue");
    EDIT_FIELD (date_of_expiry, "Date of Expiry");
    EDIT_FIELD (authority, "Authority");
    EDIT_FIELD (residence, "Residence");
    EDIT_FIELD (height, "Height");
    EDIT_FIELD (colour_of_eyes, "Colour of Eyes");

    return 1;
}
