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

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <freefare.h>
#include <ucard.h>
#include <time.h>
#include "on_password_requested.h"

#define FIDELITE_AID 0x00122344

static int      on_card_presented (struct ucard *ucard, struct ucard_application *ucard_application);
uint8_t master_key_data[] = { 0x9a, 0x4e, 0x0c, 0x0a, 0x32, 0x14, 0x44, 0xc6, 0x15, 0x07, 0xcd, 0x9f, 0xa5, 0x2b, 0x0f, 0x83 };
uint8_t public_key_data[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t user_key_data[] = { 0xd4, 0xa2, 0x3a, 0x64, 0xa8, 0x18, 0xf4, 0xc0, 0x9f, 0xc5, 0xcf, 0x55, 0x73, 0x2d, 0xdd, 0xed };

int
main (void)
{
    struct kiosk *kiosk = kiosk_new ();
    struct ucard_application *application = ucard_application_new (on_password_requested);

    ucard_application_set_aid (application, FIDELITE_AID);
    ucard_application_set_name (application, "fidelite");
    ucard_application_add_key (application, mifare_desfire_3des_key_new (user_key_data));
    ucard_application_add_key (application, mifare_desfire_des_key_new (public_key_data));
    ucard_application_add_key (application, mifare_desfire_3des_key_new (master_key_data));
    ucard_application_add_value_file (application, "point", 0x0F2F, 0, 100, 0, 0);
    ucard_application_add_std_data_file (application, "time", 0x0F2F, 20);

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
    int32_t points;
    time_t time_now = time(NULL);
    struct tm tm_now, *tm_tmp;
    tm_tmp = gmtime_r(&time_now, &tm_now);

    int res = ucard_application_select (ucard, ucard_application);
    if (res < 0) goto error;
    if (ucard_value_file_get_value (ucard, ucard_application, COUNTER_FILENO, COUNTER_RW_KEYNO, master_key, &points) < 0) goto error;
    int nb_dizaine = points/10;
    if (nb_dizaine >=1){
        int debit_value=11;
        while ( ( debit_value > nb_dizaine) ) {
            printf ("%d points de fidelite ont ete accumules\n",points);
            printf ("Combien de dizaines de points voulez-vous utiliser?\n");
            if (scanf("%d",&debit_value) < 1){
                printf("scanf failed:%s\n",strerror(errno));
            }
        }
        if (debit_value >=1){
            char lastuse[20];
            strftime(lastuse, sizeof(lastuse),"%Y-%m-%d %T",&tm_now);
            if (ucard_write_data(ucard, ucard_application, COUNTER_FILENO + 1, COUNTER_RW_KEYNO, master_key, 0, sizeof(lastuse), lastuse) < 0) goto error;
            if (ucard_value_file_debit(ucard, ucard_application, COUNTER_FILENO, COUNTER_RW_KEYNO, master_key, debit_value*10) < 0) goto error;
            if (ucard_transaction_commit (ucard) < 0) goto error;
            if (ucard_value_file_get_value (ucard, ucard_application, COUNTER_FILENO, COUNTER_RW_KEYNO, master_key, &points) < 0) goto error;
            printf("Il reste %d points\n",points);
        }

    } else {
        printf("Pas assez de points\n");
        printf("Nombre de points de fidelite : %d\n",points);
    }

    mifare_desfire_key_free (master_key);

    return 1;

error:
    printf ("Sorry! An error occured: %s\n", libucard_strerror (ucard_errno (ucard)));
    ucard_transaction_abort (ucard);

    return 0;
}
