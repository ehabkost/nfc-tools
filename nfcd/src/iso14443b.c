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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif // HAVE_CONFIG_H

#define _BSD_SOURCE
#include <endian.h>

#include <err.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <nfc/nfc.h>
#include <nfc/nfc-messages.h>

#include "iso14443b.h"

#define MAX_DEVICE_COUNT 16

struct iso14443b_tag {
    nfc_device_t *device;
    nfc_iso14443b_info_t info;
    int active;
};

/*
 * Returns the UID of the provided tag.
 */
char *
iso14443b_get_tag_uid (ISO14443bTag tag)
{
  size_t szPupi = sizeof(tag->info.abtPupi);
  char *res = malloc (2 * szPupi + 1);
  size_t i;
  for ( i =0; i < szPupi; i++)
    snprintf (res + 2*i, 3, "%02x", tag->info.abtPupi[i]);
  return res;
}
/*
 * Returns the name of the provided tag.
 */
char *
iso14443b_get_tag_name (ISO14443bTag tag)
{
  return "iso14443b";
}

ISO14443bTag
iso14443b_tag_new (void)
{
  return malloc (sizeof (struct iso14443b_tag));
}

/*
 * Free the provided tag.
 */
void
iso14443b_free_tag (ISO14443bTag tag)
{
  if (tag) {
    iso14443b_tag_free (tag);
  }
}


void
iso14443b_tag_free (ISO14443bTag tag)
{
  free (tag);
}

/*
 * Establish connection to the provided tag.
 */
int
iso14443b_connect (ISO14443bTag tag)
{
  // Poll for a ISO14443B tag
  nfc_target_t t;
  const nfc_modulation_t nm = {
    .nmt = NMT_ISO14443B,
    .nbr = NBR_106,
  };
  while (nfc_initiator_select_passive_target(tag->device, nm, (byte_t *) "\x00", 1, &t)) {
    size_t szPupi = sizeof(t.nti.nbi.abtPupi);
    if (memcmp(tag->info.abtPupi, t.nti.nbi.abtPupi, szPupi) == 0) {
      return 0;
    } else {
      nfc_initiator_deselect_target (tag->device);
    }
  }
  return -1;
}

/*
 * Terminate connection with the provided tag.
 */
int
iso14443b_disconnect (ISO14443bTag tag)
{
  if (nfc_initiator_deselect_target (tag->device)) {
      tag->active = 0;
  } else {
    return -1;
  }
  return 0;
}
