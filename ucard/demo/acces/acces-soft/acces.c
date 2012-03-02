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

#if defined(HAVE_CRYPT_H)
#  include <crypt.h>
#endif
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ucard.h>
#include <ucard_internal.h>
#include <unistd.h>
#include "on_password_requested.h"

#include "../access_file.h"

#define AUTH_FILE "/home/audrey/Projets/internal-softwares/ucard/auth.txt"
#define CRYPT_SALT "AD"
#define SCP_SCRIPT "/home/audrey/Projets/internal-softwares/ucard/scp_auth"
#define TARGET_IP " 192.168.4.103"

#define ACCES_AID 0x00144566

static int      on_card_presented ( struct ucard *ucard, struct ucard_application *ucard_application );

uint8_t master_key_data[] = { 0x78, 0x68, 0x9c, 0xf8, 0x3c, 0x14, 0xf6, 0x1c, 0x6d, 0x43, 0xd9, 0x97, 0x3b, 0xc9, 0xab, 0x43 };
uint8_t public_key_data[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t user_key_data[] = { 0xd4, 0xa2, 0x3a, 0x64, 0xa8, 0x18, 0xf4, 0xc0, 0x9f, 0xc5, 0xcf, 0x55, 0x73, 0x2d, 0xdd, 0xed };

int
main ( void )
{
    struct kiosk *kiosk = kiosk_new ();
    struct ucard_application *application = ucard_application_new (on_password_requested);
    if (application) {
        ucard_application_set_aid (application, ACCES_AID);
        ucard_application_set_name (application, "acces");
        ucard_application_add_key (application, mifare_desfire_3des_key_new (user_key_data));
        ucard_application_add_key (application, mifare_desfire_des_key_new (public_key_data));
        ucard_application_add_key (application, mifare_desfire_3des_key_new (master_key_data));
        ucard_application_add_std_data_file (application, "Name", 0x022F, 51);
        ucard_application_add_std_data_file (application, "Key", 0x022F, 51);
        ucard_application_add_value_file (application, "Access_right", 0x0F2F, 0, 1, 0, 0);
    }

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
on_card_presented ( struct ucard *ucard, struct ucard_application *ucard_application )
{
    MifareDESFireKey master_key = mifare_desfire_3des_key_new_with_version ( master_key_data );

    char buffer[BUFSIZ];
    char surname[51];
    int value;
    char scp[256];

    int res = ucard_application_select ( ucard, ucard_application );
    if ( res < 0 ) goto error;

    char *uid = freefare_get_tag_uid ( ucard->tag);
    char *access_key = crypt ( uid, CRYPT_SALT );
    if ( ucard_write_data ( ucard, ucard_application, COUNTER_FILENO + 1, COUNTER_RW_KEYNO, master_key, 0, strlen(access_key), access_key ) < 0 ) goto error;
    if ( ucard_read_data ( ucard, ucard_application, COUNTER_FILENO + 0, COUNTER_RW_KEYNO, master_key, 0, sizeof ( surname ), surname ) < 0 ) goto error;
    if ( *surname == '\0' ) {
        printf ( "Veuillez entrer le nom du propriétaire de la carte. \n" );
        if (fgets ( surname, sizeof ( surname ), stdin ) == NULL){
            printf ("fgets failed: %s\n",strerror(errno));
        }
        if ( ucard_write_data ( ucard, ucard_application, COUNTER_FILENO, COUNTER_RW_KEYNO, master_key, 0, sizeof ( surname ), surname ) < 0 ) goto error;
    } else {
        printf ( "Le proprietaire actuel de la carte est %s \n Voulez-vous modifier le nom?[oN]", surname );
        if (fgets ( buffer, BUFSIZ, stdin ) == NULL){
            printf ("fgets failed: %s\n",strerror(errno));
        }
        if ( ( buffer[0] == 'o' ) || ( buffer[0] == 'O' ) ) {
            printf ( "Veuillez entrer le nom du propriétaire de la carte. \n" );
            if (fgets ( surname, sizeof ( surname ), stdin ) == NULL){
                printf ("fgets failed: %s\n",strerror(errno));
            }
            if ( ucard_write_data ( ucard, ucard_application, COUNTER_FILENO, COUNTER_RW_KEYNO, master_key, 0, sizeof ( surname ), surname ) < 0 ) goto error;
        }
    }
    printf ( "Cette personne a-t'elle les droits d'acces?[oN]" );
    if (fgets ( buffer, BUFSIZ, stdin ) == NULL){
        printf ("fgets failed: %s\n",strerror(errno));
    }
    if ( ucard_value_file_get_value ( ucard, ucard_application, COUNTER_FILENO + 2, COUNTER_RW_KEYNO, master_key, &value ) < 0 ) goto error;
    strcpy(scp,SCP_SCRIPT);
    strcat(scp,TARGET_IP);
    if ( ( buffer[0] == 'o' ) || ( buffer[0] == 'O' ) ) {
        if ( value == 0 ) {
            if ( ucard_value_file_credit ( ucard, ucard_application, COUNTER_FILENO + 2, COUNTER_RW_KEYNO, master_key, 1 ) < 0 ) goto error;
            if ( ucard_transaction_commit ( ucard ) < 0 ) goto error;
            if ( !access_file_check ( access_key, AUTH_FILE ) ) {
                access_file_add ( access_key, AUTH_FILE );
                if (system ( scp ) <0){
                    printf("system failed:%s\n",strerror(errno));
                }
            }
        }
    } else {
        if ( value == 1 ) {
            if ( ucard_value_file_debit ( ucard, ucard_application, COUNTER_FILENO + 2, COUNTER_RW_KEYNO, master_key, 1 ) < 0 ) goto error;
            if ( ucard_transaction_commit ( ucard ) < 0 ) goto error;
            access_file_delete ( access_key, AUTH_FILE );
            if (system ( scp ) <0){
                printf("system failed:%s\n",strerror(errno));
            }
        }
    }
    mifare_desfire_key_free (master_key);

    return 1;

error:
    printf ( "Sorry! An error occured: %s\n", libucard_strerror ( ucard_errno (ucard) ) );
    ucard_transaction_abort ( ucard );

    return 0;
}
