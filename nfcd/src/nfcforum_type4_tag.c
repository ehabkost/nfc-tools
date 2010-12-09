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

#include "nfcforum_type4_tag.h"

#define CLEAR_TEXT    0x00
#define SELECT        0xa4
#define READ_BINARY   0xb0

// FIXME Remove me
#define BUF_SIZE 4096


bool
nfcforum_type4_transceive(nfc_device_t* pnd, const byte_t* dataIn, const size_t szDataIn, byte_t* dataOut, size_t* pszDataOut)
{
  bool locally_malloced = false;
  bool res = false;

  if( (!dataOut) || (!pszDataOut) ) {
    dataOut = malloc(BUF_SIZE);
    pszDataOut = malloc(sizeof(size_t));
    locally_malloced = true;
  }

  if ( nfc_initiator_transceive_bytes (pnd, dataIn, szDataIn, dataOut, pszDataOut)){
    if ( (dataOut[(*pszDataOut)-2] == 0x90) && (dataOut[(*pszDataOut)-1] == 0x00) ) {
      res = true;
    }
  }
  if( locally_malloced ) {
    free(dataOut);
    free(pszDataOut);
  }
  return res;
}

bool
nfcforum_type4_read(nfc_device_t* pnd, uint16_t offset, const uint8_t length, byte_t* data, size_t *szData)
{
  byte_t read_binary_cmd[] = { CLEAR_TEXT, READ_BINARY, 0x00, 0x00, 0x00 };
  offset = htobe16(offset);
  memcpy(read_binary_cmd + 2, &offset, sizeof (offset));
  memcpy(read_binary_cmd + 4, &length, sizeof (length));
  static byte_t SW[BUF_SIZE];
  static size_t szSWLen;

  if(!nfcforum_type4_transceive(pnd, read_binary_cmd, sizeof(read_binary_cmd), SW, &szSWLen) ) return false;
  // Save the received byte count
  *szData = szSWLen-2;

  // Copy the received bytes
  memcpy(data,SW,*szData);

  return true;
}

bool
nfcforum_type4_ndef_tag_application_select(nfc_device_t* pnd)
{
  byte_t ndef_tag_application_select_cmd[] = { CLEAR_TEXT, SELECT, 0x04, 0x00, 0x07, 0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x00 }; // See NFC Forum Type 4 tag document
  return nfcforum_type4_transceive(pnd,ndef_tag_application_select_cmd, sizeof(ndef_tag_application_select_cmd), NULL, NULL);
}

bool
nfcforum_type4_select(nfc_device_t* pnd, const byte_t fileID[2])
{
  byte_t select_cmd[] = { CLEAR_TEXT, SELECT, 0x00, 0x00, 0x02, 0x00, 0x00 };
  memcpy(select_cmd + 5, fileID, sizeof(fileID));
  return nfcforum_type4_transceive(pnd, select_cmd,sizeof(select_cmd), NULL, NULL);
}
