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
#include <string.h>
#include <unistd.h>

#include <ucard.h>

#include "transport.h"
#include "../../gpio.h"
#include <time.h>
#include "on_password_requested.h"
#define ZONE "bus_146"

/*
 * Quotting FreeBSD man page,
 *
 * The timegm() function is not specified by any standard; its function can‐
 * not be completely emulated using the standard functions described above.
 *
 * This headers cannot be visible if any _XOPEN_SOURCE is set on *BSD, and this
 * macro has to be defined for other parts of the project to build because of
 * the autotools.
 */
time_t		timegm(struct tm *tm);

static int      on_card_presented (struct ucard *ucard, struct ucard_application *ucard_application);

int
main (void)
{
    struct kiosk *kiosk = kiosk_new ();
    struct ucard_application *application = transport_application_new (on_password_requested);
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
    int32_t value;
    struct tm tm_now, *tm_tmp;
    time_t time_now = time(NULL);
    tm_tmp = gmtime_r(&time_now, &tm_now);

    if (transport_ticket_get_value (ucard, ucard_application, &value) < 0) goto error;

    if (value == 0) {
        rgb_led(0, 1, 1);
        sleep(5);
        rgb_led(0, 0, 1);
    } else {
        char lastuse[20];
        char lastzone[11];
        if (transport_time_read_data(ucard, ucard_application, 0, sizeof(lastuse), lastuse)< 0) goto error;
        if (transport_zone_read_data (ucard, ucard_application, 0, sizeof(lastzone), lastzone)< 0) goto error;
        if ((*lastuse =='\0') || (strcmp(lastzone, ZONE) != 0)){
            strftime(lastuse, sizeof(lastuse),"%Y-%m-%d %T", &tm_now);
            if (transport_ticket_debit (ucard, ucard_application, 1) < 0) goto error;
            if (transport_time_write_data (ucard, ucard_application, 0, sizeof(lastuse), lastuse)< 0) goto error;
            if (transport_zone_write_data (ucard, ucard_application, 0, sizeof(ZONE), ZONE)< 0) goto error;
            if (ucard_transaction_commit (ucard) < 0) goto error;
            rgb_led(1, 0, 1);
            sleep(5);
            rgb_led(0, 0, 1);
        } else {
            time_t time_lastuse;
            struct tm tm_lastuse;
            double deltaTime;

            time_now = timegm(&tm_now);
            strptime(lastuse,"%Y-%m-%d %T",&tm_lastuse);
            time_lastuse = timegm(&tm_lastuse);
            deltaTime = difftime(time_now, time_lastuse);

            if (deltaTime > 900){
                strftime(lastuse, sizeof(lastuse),"%Y-%m-%d %T", &tm_now);
                if (transport_ticket_debit (ucard, ucard_application, 1) < 0) goto error;
                if (transport_time_write_data (ucard, ucard_application, 0, sizeof(lastuse), lastuse)< 0) goto error;
                if (ucard_transaction_commit (ucard) < 0) goto error;
                rgb_led(1, 0, 1);
                sleep(5);
                rgb_led(0, 0, 1);
            }
        }
    }

    return 1;

error:
    printf ("Sorry! An error occured: %s\n", libucard_strerror (ucard_errno (ucard)));
    ucard_transaction_abort (ucard);

    return 0;
}
