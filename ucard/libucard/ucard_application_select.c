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
 * $Id: ucard_application_select.c 583 2011-01-01 16:09:54Z romain $
 */

#include <stdlib.h>
#include <freefare.h>

#include <ucard.h>

#include "ucard_internal.h"

int
ucard_application_select (struct ucard *ucard, const struct ucard_application *application)
{
    int res = 0;

    ucard->last_error = LIBUCARD_SUCCESS;

    if (application->aid != ucard->selected_aid) {
	MifareDESFireAID aid = mifare_desfire_aid_new (application->aid);
	if (!aid) {
	    ucard->last_error = LIBUCARD_MALLOC;
	    return -1;
	}

	res = mifare_desfire_select_application (ucard->tag, aid);

	if ((res < 0) && (APPLICATION_NOT_FOUND == mifare_desfire_last_picc_error (ucard->tag))) {

	    res = ucard_application_create (ucard, application);

	    if (res < 0) {
		ucard->last_error = UCARD_FREEFARE_ERROR;
	    } else {
		res = mifare_desfire_select_application (ucard->tag, aid);
	    }
	}
	free (aid);

	if (0 == res)
	    ucard->selected_aid = application->aid;

	ucard->current_key = -1;
    }

    return res;
}
