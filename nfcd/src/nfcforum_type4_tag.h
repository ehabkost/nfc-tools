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

#ifndef __NFCFORUM_TAG_H__
#define __NFCFORUM_TAG_H__

#include <stdint.h>

#include <nfc/nfc.h>

#ifdef __cplusplus
    extern "C" {
#endif // __cplusplus

/*
typedef union {
  MifareTag mifare;
  ISO14443bTag iso14443b;
} nfcforum_type4_tag_iso7816_t;

typedef enum {
  DESFIRE,
  ISO14443B
} nfcforum_type4_tag_type_t;

typedef struct {
  nfcforum_type4_tag_iso7816_t iso7816,
  nfcforum_type4_tag_type_t type,
} nfcforum_type4_tag_t;
*/

bool transceive(nfc_device_t* pnd, const byte_t* dataIn, const size_t szDataIn, byte_t* dataOut, size_t* pszDataOut);
bool nfcforum_type4_read(nfc_device_t* pnd, uint16_t offset, const uint8_t length, byte_t* data, size_t *szData);
bool nfcforum_type4_ndef_tag_application_select(nfc_device_t* pnd);
bool nfcforum_type4_select(nfc_device_t* pnd, const byte_t fileID[2]);

#ifdef __cplusplus
    }
#endif // __cplusplus

#endif /* __NFCFORUM_TAG_H__ */