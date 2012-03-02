/*-
 * Copyright (C) 2010, Audrey Diacre.
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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ucard.h>
#include "acces.h"
#include "../../gpio.h"
#include "../access_file.h"
#include "on_password_requested.h"
#define AUTH_FILE "/usr/auth.txt"

static int      on_card_presented (struct ucard *ucard, struct ucard_application *ucard_application);

int
main (void)
{
    struct kiosk *kiosk = kiosk_new ();
    struct ucard_application *application = acces_application_new (on_password_requested);
    init_led();

    if (kiosk_devices_scan (kiosk)) {
        kiosk_set_one_shot (kiosk, false);
        kiosk_setup (kiosk, application, on_card_presented);
        kiosk_start (kiosk);

        fd_set r;
        FD_ZERO (&r);
        FD_SET (0, &r);

        while (kiosk_select (kiosk, 0, &r, NULL, NULL, NULL) > 0) {
            if (FD_ISSET (0, &r)) {
                fprintf (stderr, "Key pressed on the console.\nExiting.\n");
                break;
            }
        }
        kiosk_stop (kiosk);
    }
    kiosk_free (kiosk);

    ucard_application_free (application);

    exit (EXIT_SUCCESS);
}

static int
on_card_presented (struct ucard *ucard, struct ucard_application *ucard_application)
{
    char access_key[51];
    int red_led;
    int green_led;
    int blue_led;

    if( acces_key_read_data(ucard, ucard_application, 0, sizeof(access_key), access_key) < 0 ) goto error;
    if ( (*access_key=='\0') || ( !access_file_check( access_key, AUTH_FILE ) )) {
        red_led = 0;
        green_led = 1;
        blue_led = 1;
    } else {
        red_led = 1;
        green_led = 0;
        blue_led = 1;
    }
    rgb_led(red_led,green_led,blue_led);
    sleep (5);
    rgb_led(0,0,1);

    return 1;

    error:
    printf ("Sorry! An error occured: %s\n", libucard_strerror (ucard_errno (ucard)));
    ucard_transaction_abort (ucard);

    return 0;
}
