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

#include <sys/types.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <freefare.h>
#include <ucard.h>
#include "on_password_requested.h"

#define GENERAL_AID 0x00000000
#include "../libucard/ucard_internal.h"

static int       application_setup (MifareTag tag, const MifareDESFireAID aid, MifareDESFireKey access_key);
static int      on_card_presented ( struct ucard *ucard, struct ucard_application *ucard_application );
uint8_t user_key_data[] = { 0xd4, 0xa2, 0x3a, 0x64, 0xa8, 0x18, 0xf4, 0xc0, 0x9f, 0xc5, 0xcf, 0x55, 0x73, 0x2d, 0xdd, 0xed };
uint8_t public_key_data[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

int
main ( void )
{
    struct kiosk *kiosk = kiosk_new ();
    struct ucard_application *application = ucard_application_new (on_password_requested);

    ucard_application_set_aid (application, GENERAL_AID);
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

static int
application_setup ( MifareTag tag, const MifareDESFireAID aid, MifareDESFireKey access_key )
{
    int res;

    (void) access_key;

    res = mifare_desfire_select_application (tag, aid);

    return res;
}
static int
on_card_presented ( struct ucard *ucard, struct ucard_application *ucard_application )
{
    int res;
    MifareDESFireAID *aids = NULL;
    uint8_t *files = NULL;
    size_t aid_count;
    size_t file_count;

    (void) ucard_application;

    MifareDESFireKey user_key = mifare_desfire_3des_key_new_with_version ( user_key_data );
    MifareTag tag = ucard_get_tag(ucard);

    res = mifare_desfire_get_application_ids(tag, &aids, &aid_count);
    printf("********************************************************\n");
    printf ("Vous avez %d comptes sur votre carte\n", (int) aid_count);
    printf("********************************************************\n");
    for (size_t i=0; i<aid_count;i++)
    {
        res = mifare_desfire_select_application ( tag, aids[i] );
        res = mifare_desfire_authenticate ( tag, 0, user_key );
        res =  mifare_desfire_get_file_ids(tag,&files,&file_count);
        char buffer[1024];
        ssize_t len = mifare_desfire_read_data (tag, UCARD_METDATA_FILENO, 0, 0, buffer);
        if (len < 0) {
            fprintf (stderr, "mifare_desfire_read_data() failed\n");
        }
        char *p = buffer;
        fprintf (stdout, "  Application \"%s\" :\n", p);
        p = strchr (p, '\0');
        p++;
        int file_no = 0;
        printf("********************************************************\n");
        while (*p) {

                if (UCARD_METDATA_FILENO == file_no)
                    file_no++;

                struct mifare_desfire_file_settings settings;
                res = mifare_desfire_get_file_settings(tag,files[file_no],&settings);
                int file_type=settings.file_type;
                if ((file_type == 0) || (file_type == 1)){
                    char data[settings.settings.standard_file.file_size];
                    res = mifare_desfire_read_data(tag,files[file_no],0,sizeof(data),data);
                    fprintf (stdout, "    %s : %s\n", p, data);
                }
                if (file_type == 2){
                    int value;
                    res = mifare_desfire_get_value(tag,files[file_no],&value);
                    fprintf (stdout, "    %s : %d\n", p, value);
                }
                if ((file_type == 3) || (file_type == 4)){
                    char record[settings.settings.linear_record_file.record_size];
                    res = mifare_desfire_read_records(tag,files[file_no],0,sizeof(record),record);
                    fprintf (stdout, "    %s : %s\n", p, record);
                }
                p = strchr (p, '\0');
                p++;
                file_no++;
        }
            printf("********************************************************\n");
    }
    if (aid_count > 0) {
        mifare_desfire_free_application_ids(aids);
    }
    mifare_desfire_key_free (user_key);
    return 1;
}
