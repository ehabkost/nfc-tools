/*-
 * NFCd
 *
 * Copyright (C) 2010, Audrey Diacre
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
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
 */

#ifndef __ISO14443B_H__
#define __ISO14443B_H__

#include <stdint.h>

#include <nfc/nfc.h>

#ifdef __cplusplus
    extern "C" {
#endif // __cplusplus

struct iso14443b_tag;
typedef struct iso14443b_tag *ISO14443bTag;

ISO14443bTag * iso14443b_get_tags (nfc_device_t* pnd);
char * iso14443b_get_tag_uid (ISO14443bTag tag);
char * iso14443b_get_tag_name (ISO14443bTag tag);
ISO14443bTag iso14443b_tag_new (void);
void iso14443b_configure (nfc_device_t* pnd);
void iso14443b_free_tag (ISO14443bTag tag);
void iso14443b_tag_free (ISO14443bTag tag);
int iso14443b_connect (ISO14443bTag tag);
int iso14443b_disconnect (ISO14443bTag tag);

#ifdef __cplusplus
    }
#endif // __cplusplus

#endif /* !__ISO14443B_H__ */