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
 * $Id: ucard-prepare.c 637M 2012-03-02 12:16:21Z (local) $
 */

#include <err.h>
#include <freefare.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <ucard.h>

static void
read_password (const char *prompt, char *buffer, size_t bufsiz)
{
    char *buffer_check = malloc (bufsiz);

    if (!bufsiz)
	err (EXIT_FAILURE, "malloc");

    do {
	fprintf (stderr, "%s: ", prompt);
	system ("stty -echo");
	fgets (buffer, bufsiz, stdin);
	system ("stty echo");
	fprintf (stderr, "\n(once more): ");
	system ("stty -echo");
	fgets (buffer_check, bufsiz, stdin);
	system ("stty echo");
	if (strcmp (buffer, buffer_check))
	    fprintf (stderr, "\nPassword mismatched!\n");
    } while (strcmp (buffer, buffer_check));

    free (buffer_check);

    char *p;
    if ((p = strchr (buffer, '\n')))
	*p = '\0';
}

void
usage (char *name)
{
    fprintf (stderr, "usage: %s [-f]\n", name);
}

int
main (int argc, char *argv[])
{
    /*
     * Collect information
     */

    bool f_flag = false;
    char *p;

    char ch;
    while ((ch = getopt (argc, argv, "f")) != -1) {
	switch (ch) {
	case 'f':
	    f_flag = true;
	    break;
	default:
	    usage (argv[0]);
	    exit (EXIT_FAILURE);
	    break;
	}
    }

    char issuer_address[BUFSIZ];
    fprintf (stderr, "Card issuer address: ");
    fgets (issuer_address, BUFSIZ, stdin);
    if ((p = strchr (issuer_address, '\n')))
	*p = '\0';

    char issuer_password[BUFSIZ];
    read_password ("Issuer password", issuer_password, BUFSIZ);
    fprintf (stderr, "\n");
    uint8_t issuer_password_data[16];
    ucard_derivate_password (issuer_password, strlen (issuer_password), 16, issuer_password_data);
    memset (issuer_password, '\0', strlen (issuer_password));
    MifareDESFireKey card_issuer_key = mifare_desfire_aes_key_new (issuer_password_data);

    char owner_full_name[BUFSIZ];
    fprintf (stderr, "Card owner full user name: ");
    fgets (owner_full_name, BUFSIZ, stdin);
    if ((p = strchr (owner_full_name, '\n')))
	*p = '\0';

    char user_password[BUFSIZ];
    read_password ("User password", user_password, BUFSIZ);
    fprintf (stderr, "\n");
    uint8_t user_key_data[16];
    ucard_derivate_password (user_password, strlen (user_password), 16, user_key_data);
    memset (user_password, '\0', strlen (user_password));
    MifareDESFireKey user_key = mifare_desfire_aes_key_new_with_version (user_key_data, UCARD_AES_KEY_VERSION);

    char admin_password[BUFSIZ];
    read_password ("Admin password", admin_password, BUFSIZ);
    fprintf (stderr, "\n");
    uint8_t admin_key_data[16];
    ucard_derivate_password (admin_password, strlen (admin_password), 16, admin_key_data);
    memset (admin_password, '\0', strlen (admin_password));

    MifareDESFireKey admin_key = mifare_desfire_aes_key_new_with_version (admin_key_data, UCARD_AES_KEY_VERSION);

    uint8_t null_key_data[16] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    MifareDESFireKey null_des_key = mifare_desfire_des_key_new (null_key_data);
    MifareDESFireKey null_aes_key = mifare_desfire_aes_key_new (null_key_data);

    nfc_connstring nfc_devices[8];
    size_t nfc_device_count;

    nfc_device_count = nfc_list_devices (NULL, nfc_devices, 8);

    for (size_t n = 0; n < nfc_device_count; n++) {
	nfc_device *nfc_device = nfc_open(NULL, nfc_devices[n]);

	MifareTag *tags = freefare_get_tags (nfc_device);
	for (int i = 0; tags[i]; i++) {
	    MifareTag tag = tags[i];
	    if (DESFIRE == freefare_get_tag_type (tag)) {

		/* Actually setup the card */

		printf ("Fond Mifare DESFire with UID: %s\n", freefare_get_tag_uid (tag));

		/*
		 * Master Application
		 *
		 * Key  0: Card owner 'user' private key
		 */

		MifareDESFireAID ucard_info_aid = mifare_desfire_aid_new (UCARD_INFO_AID);

		int res = mifare_desfire_connect (tag);
		if (f_flag) {
		    if (0 == res) res = mifare_desfire_authenticate_aes (tag, 0, user_key);
		    if (0 == res) res = mifare_desfire_format_picc (tag);
		} else {
		    if (0 == res) res = mifare_desfire_authenticate (tag, 0, null_des_key);
		    if (0 == res) res = mifare_desfire_change_key (tag, 0, user_key, NULL);
		    if (0 == res) res = mifare_desfire_authenticate_aes (tag, 0, user_key);
		}
		if (0 == res) res = mifare_desfire_create_application (tag, ucard_info_aid, 0x0F, 0x83);
		if (!f_flag) {
		    if (0 == res) res = mifare_desfire_change_key_settings (tag, 0x01);
		}

		/*
		 * Card information application
		 *
		 * Key  0: Card owner 'admin' private key
		 * Key  1: Card issuer private key
		 * Key  2: Anonymous access public key
		 *
		 * File  9: Card issuer information
		 * File 10: Card owner information
		 * File 11: Keyring
		 */

		if (0 == res) res = mifare_desfire_select_application (tag, ucard_info_aid);
		if (0 == res) res = mifare_desfire_authenticate_aes (tag, 0, null_aes_key);
		if (0 == res) res = mifare_desfire_change_key (tag, 0, admin_key, NULL);
		if (0 == res) res = mifare_desfire_authenticate_aes (tag, 0, admin_key);
		if (0 == res) res = mifare_desfire_change_key (tag, 1, card_issuer_key, NULL);

		if (0 == res) res = mifare_desfire_create_std_data_file (tag, 9, MDCM_ENCIPHERED, 0x0000, strlen (issuer_address));
		if (0 == res) res = mifare_desfire_write_data (tag, 9, 0, strlen (issuer_address), issuer_address);
		if (strlen (issuer_address) ==  (size_t) res) res = mifare_desfire_change_file_settings (tag, 9, MDCM_ENCIPHERED, 0x2F11);

		if (0 == res) res = mifare_desfire_create_std_data_file (tag, 10, MDCM_ENCIPHERED, 0x0000, strlen (owner_full_name));
		if (0 == res) res = mifare_desfire_write_data (tag, 10, 0, strlen (owner_full_name), owner_full_name);
		if (strlen (owner_full_name) == (size_t) res) res = mifare_desfire_change_file_settings (tag, 10, MDCM_ENCIPHERED, 0x0F11);

		if (0 == res) res = mifare_desfire_create_linear_record_file (tag, 11, MDCM_ENCIPHERED, 0xF20F, 3 + 16, 24);

		if (0 == res) res = mifare_desfire_change_key_settings (tag, 0x01);

		if (res < 0) {
		    fprintf (stderr, "Oops, something went wrong! (%s)\n", freefare_strerror (tag));
		}

		mifare_desfire_disconnect (tag);

		free (ucard_info_aid);
	    }
	}

	nfc_close (nfc_device);
    }

    mifare_desfire_key_free (admin_key);
    mifare_desfire_key_free (user_key);
    mifare_desfire_key_free (card_issuer_key);
    mifare_desfire_key_free (null_aes_key);
    mifare_desfire_key_free (null_des_key);

    exit(EXIT_SUCCESS);
}
