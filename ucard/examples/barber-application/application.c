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

#include <sys/types.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include <freefare.h>
#include <ucard.h>

#include "on_password_requested.h"

#define BARBER_AID 0x00112233


static int	 on_card_presented (struct ucard *ucard, struct ucard_application *ucard_application);
static int	 application_setup (MifareTag tag, const MifareDESFireAID aid, MifareDESFireKey access_key);

uint8_t master_key_data[] = { 0x98, 0x4a, 0x80, 0x66, 0x4a, 0x60, 0x2e, 0xa6, 0xe5, 0x69, 0xa1, 0xbf, 0xb7, 0x05, 0x1d, 0x5f };
uint8_t public_key_data[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

int
main (void)
{
    struct kiosk *kiosk = kiosk_new ();
    struct ucard_application *application = ucard_application_new (on_password_requested);

    ucard_application_set_aid (application, BARBER_AID);
    ucard_application_set_application_setup (application, application_setup);
    ucard_application_set_action (application, on_card_presented);

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
#define COUNTER_RW_KEYNO 0

static int
application_setup (MifareTag tag, const MifareDESFireAID aid, MifareDESFireKey access_key)
{
    int res;

    MifareDESFireKey master_key = mifare_desfire_3des_key_new_with_version (master_key_data);
    MifareDESFireKey public_key = mifare_desfire_des_key_new_with_version (public_key_data);

    res = mifare_desfire_create_application (tag, aid, 0x0f, 2);
    if (0 == res) res = mifare_desfire_select_application (tag, aid);
    if (0 == res) res = mifare_desfire_authenticate (tag, 0, public_key);
    if (0 == res) res = mifare_desfire_change_key (tag, COUNTER_RW_KEYNO, master_key, NULL);
    if (0 == res) res = mifare_desfire_authenticate (tag, COUNTER_RW_KEYNO, master_key);
    if (0 == res) res = mifare_desfire_create_value_file (tag, COUNTER_FILENO, MDCM_PLAIN, 0x1000, 0, 12, 0, 0);
    if (0 == res) res = mifare_desfire_change_key (tag, 1, access_key, NULL);

    mifare_desfire_key_free (master_key);
    mifare_desfire_key_free (public_key);

    return res;
}

static int
on_card_presented (struct ucard *ucard, struct ucard_application *ucard_application)
{
    int32_t value;

    MifareDESFireKey master_key = mifare_desfire_3des_key_new_with_version (master_key_data);
    MifareDESFireKey public_key = mifare_desfire_des_key_new_with_version (public_key_data);

    int res = ucard_application_select (ucard, ucard_application);
    if (res < 0) {
	ucard_perror (ucard, "ucard_application_select");
	goto error;
    }

    if (ucard_value_file_get_value (ucard, ucard_application, COUNTER_FILENO, COUNTER_RW_KEYNO, master_key, &value) < 0) {
	ucard_perror (ucard, "ucard_value_file_get_value");
	goto error;
    }

    if (value == 10) {
	printf ("RESET\n");
	if (ucard_value_file_debit (ucard, ucard_application, COUNTER_FILENO, COUNTER_RW_KEYNO, master_key, 10) < 0) {
	    ucard_perror (ucard, "ucard_value_file_debit");
	    goto error;
	}
    } else {
	printf ("INC (%d)\n", value);
	if (ucard_value_file_credit (ucard, ucard_application, COUNTER_FILENO, COUNTER_RW_KEYNO, master_key, 1) < 0) {
	    ucard_perror (ucard, "ucard_value_file_credit");
	    goto error;
	}
    }

    if (ucard_transaction_commit (ucard) < 0) {
	ucard_perror (ucard, "ucard_transaction_commit");
	goto error;
    }

    mifare_desfire_key_free (master_key);
    mifare_desfire_key_free (public_key);
    return 1;

error:
    ucard_transaction_abort (ucard);

    return 0;
}
