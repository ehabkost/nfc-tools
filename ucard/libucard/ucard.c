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

#include <stdlib.h>

#include <openssl/evp.h>

#include <ucard.h>

#include "ucard_internal.h"

struct ucard *
ucard_new (MifareTag tag)
{
    struct ucard *res;
    if ((res = malloc (sizeof (*res)))) {
	res->last_error = LIBUCARD_SUCCESS;
	res->tag = tag;
	res->selected_aid = 0x000000;
	res->current_key = -1;
	res->transaction_in_progress = false;
    }

    return res;
}

MifareTag
ucard_get_tag (struct ucard *ucard)
{
    return ucard->tag;
}

void
ucard_derivate_password (const char *pass, int passlen, int keylen, unsigned char *out)
{
  PKCS5_PBKDF2_HMAC_SHA1 (pass, passlen, (unsigned char *) "Universal Card", 14, 10000, keylen, out);
}
