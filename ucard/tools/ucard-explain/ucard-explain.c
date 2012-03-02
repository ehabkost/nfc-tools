/*-
 * Copyright (C) 2010-2011, Romain Tartiere.
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
#include <stdlib.h>
#include <string.h>

#include <freefare.h>
#include <ucard.h>

#define MAX_NFC_DEVICES 8

int explain_tag (MifareTag tag);
int explain_application (MifareTag tag, MifareDESFireAID aid, MifareDESFireKey key);

uint8_t admin_key_data[16];

int
main (void)
{
    nfc_connstring nfc_devices[MAX_NFC_DEVICES];
    size_t nfc_device_count;

    nfc_device_count = nfc_list_devices (NULL, nfc_devices, MAX_NFC_DEVICES);

    bool aborting = false;

    for (size_t n = 0; (!aborting) && (n < nfc_device_count); n++) {
	nfc_device *nfc_device = nfc_open (NULL, nfc_devices[n]);
	MifareTag *tags = freefare_get_tags (nfc_device);
	for (int i = 0; (!aborting) && tags[i]; i++) {
	    if (DESFIRE == freefare_get_tag_type (tags[i])) {

		fprintf (stdout, "Found UCard with UID 0x%s\n", freefare_get_tag_uid (tags[i]));

		printf ("Admin access key: ");
		char buffer[BUFSIZ];
		system ("stty -echo");
		fgets (buffer, BUFSIZ, stdin);
		system ("stty echo");
		char *p;
		if ((p = strchr (buffer, '\n')))
		    *p = '\0';

		ucard_derivate_password (buffer, strlen (buffer), 16, admin_key_data);
		memset (buffer, '\0', strlen (buffer));

		if (explain_tag (tags[i]) < 0)
		    aborting = true;
	    }
	}
	freefare_free_tags (tags);
    }

    exit((aborting) ? 1 : 0);
}

int
explain_tag (MifareTag tag)
{
    bool aborting = false;

    mifare_desfire_connect (tag);

    MifareDESFireAID ucard_info_aid = mifare_desfire_aid_new (UCARD_INFO_AID);
    if (mifare_desfire_select_application (tag, ucard_info_aid) < 0) {
	freefare_perror (tag, "mifare_desfire_select_application");
	return -1;
    }
    free (ucard_info_aid);

    MifareDESFireKey key = mifare_desfire_aes_key_new (admin_key_data);
    if (mifare_desfire_authenticate_aes (tag, 0, key) < 0) {
	freefare_perror (tag, "mifare_desfire_authenticate_aes");
	return -1;
    }

    uint8_t records[(3+16)*32];
    int records_length;
    records_length = mifare_desfire_read_records_ex (tag, USER_KEYRING_FILENO, 0, 0, records, USER_KEYRING_COMMUNICATION_MODE);
    if (records_length < 0) {
	if (BOUNDARY_ERROR == mifare_desfire_last_picc_error (tag)) {
	    warnx ("Card is empty");
	    return 0;
	}
	freefare_perror (tag, "mifare_desfire_read_records_ex");
	return -1;
    }

    for (int i = 0; (!aborting) && (i < records_length); i += (3+16)) {
	uint32_t raw_aid = records[i] << 16 | records[i+1] << 8 | records[i+2];

	MifareDESFireAID aid = mifare_desfire_aid_new (raw_aid);
	MifareDESFireKey key = mifare_desfire_des_key_new (records + i + 3);

	if (explain_application (tag, aid, key) < 0)
	    aborting = true;

	mifare_desfire_key_free (key);
	free (aid);
    }

    mifare_desfire_disconnect (tag);

    return (aborting) ? -1 : 0;
}

int
explain_application (MifareTag tag, MifareDESFireAID aid, MifareDESFireKey key)
{
    char buffer[1024];
    if (mifare_desfire_select_application (tag, aid) < 0) {
	fprintf (stderr, "mifare_desfire_select_application() failed\n");
	return -1;
    }

    if (mifare_desfire_authenticate (tag, USER_ACCESS_KEYNO, key) < 0) {
	freefare_perror (tag, "mifare_desfire_authenticate_des");
	return -1;
    }

    ssize_t len = mifare_desfire_read_data_ex (tag, UCARD_METDATA_FILENO, 0, 0, buffer, USER_ACCESS_COMMUNICATION_MODE);
    if (len < 0) {
	fprintf (stderr, "mifare_desfire_read_data_ex() failed\n");
	return -1;
    }

    char *p = buffer;

    fprintf (stdout, "  Application \"%s\" with AID 0x%06x has the following files:\n",
	     p,
	     mifare_desfire_aid_get_aid (aid));

    p = strchr (p, '\0');
    p++;

    int file_no = 0;

    while (*p) {
	fprintf (stdout, "    File %2d - %s\n", file_no++, p);
	p = strchr (p, '\0');
	p++;

	if (UCARD_METDATA_FILENO == file_no)
	    file_no++;
    }

    return 0;
}
