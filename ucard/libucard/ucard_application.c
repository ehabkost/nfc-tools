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
 * $Id: ucard_application.c 642 2011-01-16 12:08:19Z romain $
 */

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#include <ucard.h>

#include "ucard_internal.h"

struct ucard_application *
ucard_application_new (password_request_callback on_password_requested)
{
    struct ucard_application *res = malloc (sizeof (*res));

    if (res) {
	res->aid = 0;
	res->name = NULL;
	res->on_password_requested = on_password_requested;
	res->application_setup = NULL;
	res->on_ucard = NULL;
	res->keys.count = 0;
	res->keys.keys = NULL;
	res->files.count = 0;
	res->files.files = NULL;
    }

    return res;
}

void
ucard_application_free (struct ucard_application *application)
{
    for (size_t n = 0; n < application->keys.count; n++)
	free (application->keys.keys[n]);
    free (application->keys.keys);

    for (size_t n = 0; n < application->files.count; n++)
	free (application->files.files[n].name);
    free (application->files.files);
    free (application->name);
    free (application);
}



void
ucard_application_set_aid (struct ucard_application *application, const uint32_t aid)
{
    application->aid = aid;
}

uint32_t
ucard_application_get_aid (const struct ucard_application *application)
{
    return application->aid;
}

void
ucard_application_set_name (struct ucard_application *application, const char *name)
{
    free (application->name);
    application->name = strdup (name);
}

const char *
ucard_application_get_name (const struct ucard_application *application)
{
    return application->name;
}

void
ucard_application_set_application_setup (struct ucard_application *application, ucard_application_setup_callback application_setup)
{
    application->application_setup = application_setup;
}

void
ucard_application_set_action (struct ucard_application *application, ucard_presented_callback on_ucard)
{
    application->on_ucard = on_ucard;
}

int
ucard_application_add_key (struct ucard_application *application, const MifareDESFireKey key)
{
    MifareDESFireKey *p;

    if ((p = realloc (application->keys.keys, (application->keys.count + 1) * sizeof (*p)))) {
	application->keys.keys = p;
	application->keys.keys[application->keys.count++] = key;
    }

    return (p) ? 0 : -1;
}

MifareDESFireKey
ucard_application_get_key (const struct ucard_application *application, const uint8_t key_no)
{
    return application->keys.keys[key_no];
}



/*
 * Create the application meta-data file.
 *
 * Format is:
 * <application name> '\0' [ '\<file-number>' <file-name> '\0' [...]] '\0'
 *
 */
int
ucard_application_save_metadata (struct ucard *ucard, const struct ucard_application *application)
{
    ucard->last_error = LIBUCARD_SUCCESS;

    int res;

    size_t len = strlen (application->name) + 1;
    for (size_t n = 0; n < application->files.count; n++) {
	len += strlen (application->files.files[n].name) + 1;
    }
    len += 1;

    char *buffer = malloc (len);

    if (!buffer) {
	ucard->last_error = LIBUCARD_MALLOC;
	return -1;
    }

    char *p = buffer;
    size_t l = strlen (application->name);
    memcpy (p, application->name, l);
    p += l;
    *p++ = '\0';
    for (size_t n = 0; n < application->files.count; n++) {
	l = strlen (application->files.files[n].name);
	memcpy (p, application->files.files[n].name, l);
	p += l;
	*p++ = '\0';
    }
    *p++ = '\0';

    res = ucard_application_select (ucard, application);
    if (0 == res) res = ucard_authenticate (ucard, 0, application->keys.keys[0]);

    if (0 == res) res = mifare_desfire_create_std_data_file (ucard->tag, UCARD_METDATA_FILENO, USER_ACCESS_COMMUNICATION_MODE, 0x0000, len);
    if (0 == res) res = mifare_desfire_write_data_ex (ucard->tag, UCARD_METDATA_FILENO, 0, len, buffer, USER_ACCESS_COMMUNICATION_MODE);
    if (len == (size_t) res) res = mifare_desfire_change_file_settings (ucard->tag, UCARD_METDATA_FILENO, USER_ACCESS_COMMUNICATION_MODE, 0x1FFF);

    free (buffer);

    return res;
}
