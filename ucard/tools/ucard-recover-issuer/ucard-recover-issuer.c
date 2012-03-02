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
 * $Id: ucard-recover-issuer.c 580M 2012-03-02 12:16:55Z (local) $
 */

#include <err.h>
#include <freefare.h>
#include <string.h>
#include <stdlib.h>

#include <ucard.h>

int
main (void)
{
    uint8_t null_key_data[16] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    MifareDESFireKey null_aes_key = mifare_desfire_aes_key_new (null_key_data);

    nfc_connstring nfc_devices[8];
    size_t nfc_device_count;

    nfc_device_count = nfc_list_devices (NULL, nfc_devices, 8);

    for (size_t n = 0; n < nfc_device_count; n++) {
	nfc_device *nfc_device = nfc_open (NULL, nfc_devices[n]);

	MifareTag *tags = freefare_get_tags (nfc_device);
	for (int i = 0; tags[i]; i++) {
	    MifareTag tag = tags[i];
	    if (DESFIRE == freefare_get_tag_type (tag)) {

		printf ("Fond Mifare DESFire with UID: %s\n", freefare_get_tag_uid (tag));

		MifareDESFireAID ucard_info_aid = mifare_desfire_aid_new (UCARD_INFO_AID);

		int res = mifare_desfire_connect (tag);

		res = mifare_desfire_select_application (tag, ucard_info_aid);
		if (res < 0) {
		    freefare_perror (tag, "mifare_desfire_select_application");
		    goto end;
		}
		res = mifare_desfire_authenticate_aes (tag, 2, null_aes_key);
		if (res < 0) {
		    freefare_perror (tag, "mifare_desfire_authenticate_aes");
		    goto end;
		}

		char issuer_info[BUFSIZ];
		res = mifare_desfire_read_data_ex (tag, 9, 0, 0, issuer_info, MDCM_ENCIPHERED);
		if (res < 0) {
		    freefare_perror (tag, "mifare_desfire_read_data");
		    goto end;
		}

		printf ("Issuer: %s\n", issuer_info);

end:
		mifare_desfire_disconnect (tag);

		free (ucard_info_aid);
	    }
	}

	nfc_close (nfc_device);
    }

    mifare_desfire_key_free (null_aes_key);

    exit(EXIT_SUCCESS);
}
