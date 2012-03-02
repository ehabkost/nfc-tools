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
 * $Id: application.c 583 2011-01-01 16:09:54Z romain $
 */

#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <ucard.h>

#include "on_password_requested.h"

#include "cardlock.h"

static int	 on_card_presented (struct ucard *ucard, struct ucard_application *ucard_application);

bool grant_access = false;

int
main (int argc, char *argv[])
{
    if (argc != 2) {
	fprintf (stderr, "usage: %s <deny|grant>\n", argv[0]);
	exit (EXIT_FAILURE);
    }

    if (0 == strcmp (argv[1], "grant"))
	grant_access = true;

    struct kiosk *kiosk = kiosk_new ();
    struct ucard_application *application = cardlock_application_new (on_password_requested);

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

static int
on_card_presented (struct ucard *ucard, struct ucard_application *ucard_application)
{
    time_t t = time (NULL);
    char *buffer = ctime (&t);

    if (grant_access) {
	if (cardlock_access_grant_log_write_record (ucard, ucard_application, 0, 26, buffer) < 0) {
	    ucard_perror (ucard, "cardlock_access_grant_log_write_record");
	    goto error;
	}
	if (cardlock_access_deny_log_clear (ucard, ucard_application) < 0) {
	    ucard_perror (ucard, "cardlock_access_deny_log_clear");
	    goto error;
	}
    } else {
	if (cardlock_access_deny_log_write_record (ucard, ucard_application, 0, 26, buffer) < 0) {
	    ucard_perror (ucard, "cardlock_access_deny_log_write_record");
	    goto error;
	}
    }

    if (ucard_transaction_commit (ucard) < 0) {
	ucard_perror (ucard, "ucard_transaction_commit");
	goto error;
    }

    return 1;

error:
    ucard_transaction_abort (ucard);

    return 0;
}
