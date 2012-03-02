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

#include <sys/types.h>

#include <freefare.h>
#include <ucard.h>

#include "ucard_internal.h"

int
ucard_value_file_get_value (struct ucard *ucard, const struct ucard_application *application, const uint8_t file_no, const uint8_t key_no, MifareDESFireKey key, int32_t *value)
{
    ucard->last_error = LIBUCARD_SUCCESS;

    int res;

    if (!application)
	return -1;

    if (!value)
	return -1;

    res = ucard_application_select (ucard, application);
    if (0 == res) res = ucard_authenticate (ucard, key_no, key);
    if (0 == res) res = mifare_desfire_get_value (ucard->tag, file_no, value);

    if (res < 0) {
	ucard->last_error = UCARD_FREEFARE_ERROR;
    }

    return res;
}
