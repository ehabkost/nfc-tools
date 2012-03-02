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
 * $Id: ucard_error.c 565 2010-12-24 00:40:51Z romain $
 */

#include <string.h>
#include <stdio.h>
#include <ucard.h>

#include "ucard_internal.h"

int
ucard_errno (struct ucard *ucard)
{
    return ucard->last_error;
}

void
ucard_perror (struct ucard *ucard, const char *message)
{
    fprintf (stderr, "%s: %s\n", message, libucard_strerror (ucard_errno (ucard)));
}

int
kiosk_errno (struct kiosk *kiosk)
{
    return kiosk->last_error;
}

void
kiosk_perror (struct kiosk *kiosk, const char *message)
{
    fprintf (stderr, "%s: %s\n", message, libucard_strerror (kiosk_errno (kiosk)));
}

const char *
libucard_strerror (int errnum)
{
    switch ((enum libucard_error) errnum) {
    case LIBUCARD_SUCCESS:
	return "Success";
	break;
    case LIBUCARD_INVALID_ARGUMENT:
	return "Invalid argument";
	break;
    case LIBUCARD_MALLOC:
	return "Out of memory";
	break;
    case UCARD_FREEFARE_ERROR:
	return "Mifare DESFire error";
	break;
    case KIOSK_NO_DEVICE:
	return "Kiosk has no configured devices";
	break;
    case KIOSK_BUSY:
	return "Kiosk is busy";
	break;
    case KIOSK_PIPE:
	return "Can't create pipes";
	break;
    case KIOSK_INCONSISTENT:
	return "Kiosk devices configuration in inconsistent";
	break;
    default:
	return "Unknown error";
	break;
    }
}

char *
libucard_strerror_r (const int errnum, char *strbuf, const size_t buflen)
{
    const char *str = libucard_strerror (errnum);
    if (!str)
	return NULL;
    return strncpy (strbuf, str, buflen);
}
