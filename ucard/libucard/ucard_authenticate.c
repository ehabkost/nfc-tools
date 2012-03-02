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
ucard_authenticate (struct ucard *ucard, const uint8_t key_no, MifareDESFireKey key)
{
    ucard->last_error = LIBUCARD_SUCCESS;

    if (!key) {
	ucard->last_error = LIBUCARD_INVALID_ARGUMENT;
	return -1;
    }

    if (ucard->current_key == key_no)
	return 0;

    /*
     * mifare_desfire_authenticate() will actualy uthenticate accotding to the
     * key type.
     */
    if (mifare_desfire_authenticate (ucard->tag, key_no, key) < 0) {
	ucard->last_error = UCARD_FREEFARE_ERROR;
	return -1;
    }

    ucard->current_key = key_no;

    return 0;
}
