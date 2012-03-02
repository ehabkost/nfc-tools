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

#include <stdio.h>
#include <stdlib.h>

#include <freefare.h>
#include <ucard.h>
#include "on_password_requested.h"

#define TRANSPORT_AID 0x00112233

static int      on_card_presented (struct ucard *ucard, struct ucard_application *ucard_application);
uint8_t master_key_data[] = { 0x9a, 0x4e, 0x0c, 0x0a, 0x32, 0x14, 0x44, 0xc6, 0x15, 0x07, 0xcd, 0x9f, 0xa5, 0x2b, 0x0f, 0x83 };
uint8_t public_key_data[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t user_key_data[] = { 0xd4, 0xa2, 0x3a, 0x64, 0xa8, 0x18, 0xf4, 0xc0, 0x9f, 0xc5, 0xcf, 0x55, 0x73, 0x2d, 0xdd, 0xed };


int
main (void)
{
    struct kiosk *kiosk = kiosk_new ();
    struct ucard_application *application = ucard_application_new (on_password_requested);

    ucard_application_set_aid (application, TRANSPORT_AID);
    ucard_application_set_name (application, "transport");
    ucard_application_add_key (application, mifare_desfire_3des_key_new (user_key_data));
    ucard_application_add_key (application, mifare_desfire_des_key_new (public_key_data));
    ucard_application_add_key (application, mifare_desfire_3des_key_new (master_key_data));
    ucard_application_add_value_file (application, "ticket", 0x0F2F, 0, 10, 0, 0);
    ucard_application_add_std_data_file (application, "time", 0x0F2F, 20);
    ucard_application_add_std_data_file (application, "zone", 0x0F2F, 11);

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

#define COUNTER_FILENO 0
#define COUNTER_RW_KEYNO 3

static int
on_card_presented (struct ucard *ucard, struct ucard_application *ucard_application)
{

    MifareDESFireKey master_key = mifare_desfire_3des_key_new_with_version (master_key_data);

    int res = ucard_application_select (ucard, ucard_application);
    if (res < 0) goto error;
    if ( ucard_value_file_credit ( ucard, ucard_application, COUNTER_FILENO, COUNTER_RW_KEYNO, master_key, 10 ) < 0 ) goto error;

    if (ucard_transaction_commit (ucard) < 0) goto error;

    mifare_desfire_key_free (master_key);

    return 1; 

error:
    printf ("Sorry! An error occured: %s\n", libucard_strerror (ucard_errno (ucard)));
    ucard_transaction_abort (ucard);

    return 0;
}
