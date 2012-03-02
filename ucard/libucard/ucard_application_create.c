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
 * $Id: ucard_application_create.c 641 2011-01-16 12:07:05Z romain $
 */

#include <openssl/rand.h>
#include <string.h>
#include <stdlib.h>
#include <freefare.h>

#include <ucard.h>

#include "ucard_internal.h"

static uint8_t null_key_data[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static int
ucard_try_application_create (struct ucard *ucard, const struct ucard_application *application, MifareDESFireAID aid, MifareDESFireKey access_key)
{

	if (mifare_desfire_select_application (ucard->tag, aid) < 0) {
	    ucard->last_error = UCARD_FREEFARE_ERROR;
	    return -1;
	}

	uint8_t default_key_data[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	MifareDESFireKey default_key = mifare_desfire_des_key_new (default_key_data);
	if (!default_key) {
	    ucard->last_error = LIBUCARD_MALLOC;
	    return -1;
	}

	if (mifare_desfire_authenticate (ucard->tag, 0, default_key) < 0) {
	    ucard->last_error = UCARD_FREEFARE_ERROR;
	    free (default_key);
	    return -1;
	}
	free (default_key);

	// Setup keys
	if (mifare_desfire_change_key (ucard->tag, 0, application->keys.keys[0], NULL) < 0) {
	    ucard->last_error = UCARD_FREEFARE_ERROR;
	    return -1;
	}

	if (mifare_desfire_authenticate (ucard->tag, 0, application->keys.keys[0]) < 0) {
	    ucard->last_error = UCARD_FREEFARE_ERROR;
	    return -1;
	}

	if (mifare_desfire_change_key (ucard->tag, USER_ACCESS_KEYNO, access_key, NULL) < 0) {
	    ucard->last_error = UCARD_FREEFARE_ERROR;
	    return -1;
	}

	for (size_t i = 2; i < application->keys.count; i++) {
	    if (mifare_desfire_change_key (ucard->tag, (uint8_t) i, application->keys.keys[i], NULL) < 0) {
		ucard->last_error = UCARD_FREEFARE_ERROR;
		return -1;
	    }
	}

	for (size_t i =0; i < application->files.count; i++) {
	    uint8_t file_number = (uint8_t) i;
	    if (file_number >= UCARD_METDATA_FILENO)
		file_number++;

	    struct ucard_application_file file = application->files.files[i];
	    int res;
	    switch (file.type) {
	    case std_data_file:
		res = mifare_desfire_create_std_data_file (ucard->tag, file_number, MDCM_ENCIPHERED,
							   file.access_rights,
							   file.properties.data_file.file_size);
		break;
	    case backup_data_file:
		res = mifare_desfire_create_backup_data_file (ucard->tag, file_number, MDCM_ENCIPHERED,
							      file.access_rights,
							      file.properties.data_file.file_size);
		break;
	    case value_file:
		res = mifare_desfire_create_value_file (ucard->tag, file_number, MDCM_ENCIPHERED,
							file.access_rights,
							file.properties.value_file.lower_limit,
							file.properties.value_file.upper_limit,
							file.properties.value_file.value,
							file.properties.value_file.limited_credit_enabled);
		break;
	    case linear_record_file:
		res = mifare_desfire_create_linear_record_file (ucard->tag, file_number, MDCM_ENCIPHERED,
								file.access_rights,
								file.properties.record_file.record_size,
								file.properties.record_file.record_count);
		break;
	    case cyclic_record_file:
		res = mifare_desfire_create_cyclic_record_file (ucard->tag, file_number, MDCM_ENCIPHERED,
								file.access_rights,
								file.properties.record_file.record_size,
								file.properties.record_file.record_count);
		break;
	    }

	    if (res < 0) {
		ucard->last_error = LIBUCARD_MALLOC;
		return -1;
	    }
	}

	if (ucard_application_save_metadata (ucard, application) < 0)
	    return -1;

    return 0;
}

int
ucard_application_create (struct ucard *ucard, const struct ucard_application *application)
{
    ucard->last_error = LIBUCARD_SUCCESS;

    int res;

    uint8_t access_key_data[16];
    RAND_bytes (access_key_data, sizeof (access_key_data));
    MifareDESFireKey access_key = mifare_desfire_des_key_new (access_key_data);

    MifareDESFireAID aid = mifare_desfire_aid_new (application->aid);
    if (!aid) {
	ucard->last_error = LIBUCARD_MALLOC;
	return -1;
    }

    if (!application->on_password_requested) {
	ucard->last_error = LIBUCARD_INVALID_ARGUMENT;
	return -1;
    }

    char password[BUFSIZ];
    uint8_t user_key_data[16];
    application->on_password_requested ("User password", password, BUFSIZ);
    ucard_derivate_password (password, strlen (password), 16, user_key_data);
    memset (password, '\0', BUFSIZ);

    MifareDESFireKey user_key = mifare_desfire_aes_key_new (user_key_data);
    memset (user_key_data, '\0', sizeof (user_key_data));
    res = mifare_desfire_authenticate_aes (ucard->tag, 0, user_key);

    if (res < 0) {
	ucard->last_error = UCARD_FREEFARE_ERROR;
	return -1;
    }

    if (application->application_setup) {
	res = application->application_setup (ucard->tag, aid, access_key);
    } else {

	if (mifare_desfire_create_application (ucard->tag, aid, 0x0F, application->keys.count) < 0) {
	    ucard->last_error = UCARD_FREEFARE_ERROR;
	    return -1;
	}

	res = ucard_try_application_create (ucard, application, aid, access_key);
    }

    if (res == 0) {
	// Register the application aid and key in the user Keyring
	uint8_t record[3+16];
	record[0] = application->aid >> 16;
	record[1] = application->aid >> 8;
	record[2] = application->aid;
	memcpy (record + 3, access_key_data, 16);

	MifareDESFireAID ucard_info_aid = mifare_desfire_aid_new (UCARD_INFO_AID);

	res = mifare_desfire_select_application (ucard->tag, ucard_info_aid);
	MifareDESFireKey null_aes_key = mifare_desfire_aes_key_new (null_key_data);
	res = mifare_desfire_authenticate_aes (ucard->tag, 2, null_aes_key);
	res = mifare_desfire_write_record_ex (ucard->tag, USER_KEYRING_FILENO, 0, sizeof (record), record, USER_KEYRING_COMMUNICATION_MODE);
	res = mifare_desfire_commit_transaction (ucard->tag);
	free (ucard_info_aid);

	mifare_desfire_key_free (null_aes_key);
    } else {
	int recover_res = mifare_desfire_select_application (ucard->tag, NULL);
	if (0 == recover_res) recover_res = mifare_desfire_authenticate (ucard->tag, 0, user_key);
	if (0 == recover_res) recover_res = mifare_desfire_delete_application (ucard->tag, aid);
	if (recover_res < 0) {
	    // FIXME: This is a critical failure!
	    // Application was created, something went wrong, and the
	    // application can't be removed.
	    res = -2;
	}
    }

    mifare_desfire_key_free (user_key);

    return res;
}
