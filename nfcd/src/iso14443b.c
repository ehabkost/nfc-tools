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

#ifdef HAVE_LIBUSB
#  ifdef DEBUG
#    include <sys/param.h>
#    include <usb.h>
#  endif
#endif

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

void
iso14443b_configure (nfc_device_t* pnd)
{
  nfc_initiator_init(pnd);

  // Drop the field for a while
  nfc_configure(pnd,NDO_ACTIVATE_FIELD,false);

  // Let the reader only try once to find a tag
  nfc_configure(pnd,NDO_INFINITE_SELECT,false);

  // Configure the CRC and Parity settings
  nfc_configure(pnd,NDO_HANDLE_CRC,true);
  nfc_configure(pnd,NDO_HANDLE_PARITY,true);

  // Enable field so more power consuming cards can power themselves up
  nfc_configure(pnd,NDO_ACTIVATE_FIELD,true);
}

// FIXME Use nfc_initiator_list_passive_target()
ISO14443bTag *
iso14443b_get_tags (nfc_device_t* pnd)
{
  ISO14443bTag *tags = NULL;
  int tag_count = 0;

  iso14443b_configure (pnd);

  // Poll for a ISO14443B tag
  nfc_target_info_t target_info;

  tags = malloc(sizeof (void *));
  if(!tags) return NULL;
  tags[0] = NULL;

  while (nfc_initiator_select_passive_target(pnd, NM_ISO14443B_106, (byte_t *) "\x00", 1, &target_info)) {
    tag_count++;

    /* (Re)Allocate memory for the found ISO14443B targets array */
    ISO14443bTag *p = realloc (tags, (tag_count + 1) * sizeof (ISO14443bTag));
    if (p) {
      tags = p;
    } else {
      return tags; // FAIL! Return what has been found so far.
    }
    /* Allocate memory for the found ISO14443B target */
    tags[tag_count-1] = iso14443b_tag_new();
    if (!tags[tag_count-1]) {
      return tags; // FAIL! Return what has been found before.
    }
    (tags[tag_count-1])->device = pnd;
    (tags[tag_count-1])->info = target_info.nbi;
    (tags[tag_count-1])->active = 0;
    tags[tag_count] = NULL;

    nfc_initiator_deselect_target (pnd);
  }
  return tags;
}

/*
 * Returns the UID of the provided tag.
 */
char *
iso14443b_get_tag_uid (ISO14443bTag tag)
{
  size_t szAtqb = sizeof(tag->info.abtAtqb);
  char *res = malloc (2 * szAtqb + 1);
  size_t i;
  for ( i =0; i < szAtqb; i++)
    snprintf (res + 2*i, 3, "%02x", tag->info.abtAtqb[i]);
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
  byte_t* pupi = tag->info.abtAtqb;

  iso14443b_configure(tag->device);

  // Poll for a ISO14443B tag
  nfc_target_info_t target_info;

  while (nfc_initiator_select_passive_target(tag->device, NM_ISO14443B_106, (byte_t *) "\x00", 1, &target_info)) {
    size_t szabtAtqb = sizeof(target_info.nbi.abtAtqb);
    if (memcmp(pupi,target_info.nbi.abtAtqb,szabtAtqb) == 0) {
      return 0;
    } else {
      nfc_initiator_deselect_target (tag->device);
    }
  }
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
