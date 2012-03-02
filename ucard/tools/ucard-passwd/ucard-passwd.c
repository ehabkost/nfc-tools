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
 * $Id$
 */

#include <err.h>
#include <freefare.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <ucard.h>

void
usage (char *prog)
{
    fprintf (stderr, "usage: %s [-a]\n", prog);
}

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


int
main (int argc, char *argv[])
{
    char ch;
    int change_admin_key = 0;

    char *prog = argv[0];

    while ((ch = getopt (argc, argv, "a")) != -1) {
	switch (ch) {
	case 'a':
	    change_admin_key = 1;
	    break;
	default:
	    usage (prog);
	    exit (EXIT_FAILURE);
	}
    }
    argc -= optind;
    argv += optind;

    nfc_connstring nfc_devices[8];
    size_t nfc_device_count;

    nfc_device_count = nfc_list_devices (NULL, nfc_devices, 8);

    for (size_t n = 0; n < nfc_device_count; n++) {
	nfc_device *nfc_device = nfc_open (NULL, nfc_devices[n]);

	MifareTag *tags = freefare_get_tags (nfc_device);
	for (int i = 0; tags[i]; i++) {
	    MifareTag tag = tags[i];
	    if (DESFIRE == freefare_get_tag_type (tag)) {
		uint8_t old_key_data[16];
		uint8_t new_key_data[16];

		printf ("Fond Mifare DESFire with UID: %s\n", freefare_get_tag_uid (tag));

		printf ("Current user key: ");
		char buffer[BUFSIZ];
		system ("stty -echo");
		fgets (buffer, BUFSIZ, stdin);
		system ("stty echo");
		char *p;
		if ((p = strchr (buffer, '\n')))
		    *p = '\0';
		ucard_derivate_password (buffer, strlen (buffer), 16, old_key_data);
		memset (buffer, '\0', strlen (buffer));
		MifareDESFireKey old_key = mifare_desfire_aes_key_new (old_key_data);
		memset (old_key_data, '\0', sizeof (old_key_data));
		printf("\n");

		read_password ("New user key", buffer, BUFSIZ);
		if ((p = strchr (buffer, '\n')))
		    *p = '\0';
		ucard_derivate_password (buffer, strlen (buffer), 16, new_key_data);
		memset (buffer, '\0', strlen (buffer));
		MifareDESFireKey new_key = mifare_desfire_aes_key_new (new_key_data);
		memset (new_key_data, '\0', sizeof (new_key_data));
		printf("\n");

		int res = mifare_desfire_connect (tag);

		if (change_admin_key) {
		    MifareDESFireAID ucard_info_aid = mifare_desfire_aid_new (UCARD_INFO_AID);
		    res = mifare_desfire_select_application (tag, ucard_info_aid);
		    if (res < 0) {
			freefare_perror (tag, "mifare_desfire_select_application");
			goto end;
		    }
		    free (ucard_info_aid);
		}

		res = mifare_desfire_authenticate_aes (tag, 0, old_key);
		if (res < 0) {
		    freefare_perror (tag, "mifare_desfire_authenticate_aes");
		    goto end;
		}

		res = mifare_desfire_change_key (tag, 0, new_key, old_key);
		if (res < 0) {
		    freefare_perror (tag, "mifare_desfire_change_key");
		    goto end;
		}

end:
		mifare_desfire_disconnect (tag);

		mifare_desfire_key_free (old_key);
		mifare_desfire_key_free (new_key);
	    }
	}

	nfc_close (nfc_device);
    }

    exit(EXIT_SUCCESS);
}
