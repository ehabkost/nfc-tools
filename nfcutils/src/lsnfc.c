 /**
 * NFC utils - lsnfc
 *
 * Copyright (C) 2009, 2010, Romuald Conty
 * 
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>
 */

/*
 * This implementation was written based on information provided by the
 * following documents:
 *
 * MIFARE Type Identification Procedure
 *  AN10833
 *  Rev. 3.1 — 07 July 2009
 *  Application note
 *
 * ISO14443 tags list
 *  http://www.libnfc.org/documentation/hardware/tags/iso14443
 */
#include "config.h"

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include <string.h>

#include <nfc/nfc.h>

static nfc_device_t *pnd;

#define ERR(x, ...) printf("ERROR: " x "\n", __VA_ARGS__ )

#define MAX_DEVICE_COUNT  16
#define MAX_TARGET_COUNT  16
#define MAX_ATS_LENGTH    32

typedef char* (*identication_hook)(const nfc_iso14443a_info_t nai);

struct iso14443a_tag {
    uint8_t SAK;
    const char *name;
    size_t ATS_length;
    uint8_t ATS[MAX_ATS_LENGTH];
    identication_hook identication_fct;
};

void
print_hex (const byte_t* pbtData, size_t szData)
{
  for (size_t i = 0; i < szData; i++) {
    printf ("%02x", pbtData[i]);
  }
}

char* 
mifare_ultralight_identification(const nfc_iso14443a_info_t nai)
{
  byte_t abtCmd[2];
  byte_t abtRx[265];
  size_t szRxLen;

  abtCmd[0] = 0x30;  // MIFARE Ultralight READ command
  abtCmd[1] = 0x10;  // block address (1K=0x00..0x39, 4K=0x00..0xff)

  if(nfc_initiator_select_passive_target(pnd, NM_ISO14443A_106, nai.abtUid, nai.szUidLen, NULL) ) {
    if (nfc_initiator_transceive_bytes(pnd, abtCmd,sizeof(abtCmd), abtRx, &szRxLen)) {
      // READ command of 0x10 success, we consider that Ultralight does have 0x10 address, so it's a Ultralight C
      return strdup(" C");
    } else {
      // When a READ failed, the tag returns in HALT state, so we don't need to deselect tag
      return NULL;
    }
  } else {
    // Unable to reselect Ultralight tag
    return NULL;
  }
  nfc_initiator_deselect_target(pnd);
  return NULL;
}

/*
Document used to code this function:
   NXP Semiconductors
   AN094533
   DESFire EV1- Features and Hints
*/
char*
mifare_desfire_identification(const nfc_iso14443a_info_t nai)
{
  byte_t abtCmd[] = { 0x60 }; // MIFARE DESFire GetVersion command
  byte_t abtRx[265];
  size_t szRxLen;
  byte_t abtDESFireVersion[14];
  char* res = NULL;
  
  if(nfc_initiator_select_passive_target(pnd, NM_ISO14443A_106, nai.abtUid, nai.szUidLen, NULL) ) {
    if (nfc_initiator_transceive_bytes(pnd, abtCmd, sizeof(abtCmd), abtRx, &szRxLen)) {
      // MIFARE DESFire GetVersion command success, decoding...
      if( szRxLen == 8 ) { // GetVersion should reply 8 bytes
        memcpy( abtDESFireVersion, abtRx + 1, 7 );
        abtCmd[0] = 0xAF; // ask for GetVersion next bytes
        if (nfc_initiator_transceive_bytes(pnd, abtCmd, sizeof(abtCmd), abtRx, &szRxLen)) {
          if( szRxLen == 8 ) { // GetVersion should reply 8 bytes
            memcpy( abtDESFireVersion + 7, abtRx + 1, 7 );
            res = malloc(16); // We can alloc res: we will be able to provide information
            bool bEV1 = ( ( abtDESFireVersion[3] == 0x01 ) && ( abtDESFireVersion[10] == 0x01 ) ); // Hardware major version and software major version should be equals to 1 to be an DESFire EV1
            snprintf(res, 16, " %s%dk", bEV1 ? "EV1 " : "", (uint16_t)(1 << ((abtDESFireVersion[5] >> 1) - 10)));
          }
        }
      }
    }
  } else {
    // Select failed, tag may have been removed, so we can't provide more info and we don't have to deselect tag
    return res;
  }
  nfc_initiator_deselect_target(pnd);
  return res;
}

struct iso14443a_tag iso14443a_tags[] = {
    { 0x00, "NXP MIFARE UltraLight",      0, { 0 }, mifare_ultralight_identification },
    { 0x09, "NXP MIFARE Mini",            0, { 0 }, NULL },
    { 0x08, "NXP MIFARE Classic 1k",      0, { 0 }, NULL },
    { 0x18, "NXP MIFARE Classic 4k",      0, { 0 }, NULL },
    { 0x20, "NXP MIFARE DESFire",         5, { 0x75, 0x77, 0x81, 0x02, 0x80 }, mifare_desfire_identification },
    
    { 0x08, "NXP MIFARE Plus 1k",         0, { 0 }, NULL },
    { 0x18, "NXP MIFARE Plus 4k",         0, { 0 }, NULL },
    { 0x10, "NXP MIFARE Plus 1k",         0, { 0 }, NULL },
    { 0x11, "NXP MIFARE Plus 4k",         0, { 0 }, NULL },
    { 0x20, "NXP MIFARE Plus 1k",         0, { 0 }, NULL },
    { 0x20, "NXP MIFARE Plus 4k",         0, { 0 }, NULL },
    { 0x20, "NXP MIFARE Plus 1k/4k",     11, { 0x75, 0x77, 0x80, 0x02, 0xc1, 0x05, 0x2f, 0x2f, 0x01, 0xbc, 0xd6}, NULL },
    { 0x20, "NXP MIFARE Plus 2k/4k",     11, { 0x75, 0x77, 0x80, 0x02, 0xc1, 0x05, 0x2f, 0x2f, 0x01, 0xbc, 0xd6 }, NULL },

    { 0x88, "Infineon MIFARE Classic 1k", 0, { 0 }, NULL },
    { 0x38, "Nokia MIFARE Classic 4k (Emulated)", 0, { 0 }, NULL },
    { 0x28, "NXP JCOP31",                 0, { 0 }, NULL },
    /* @todo handle ATS to be able to know which one is it. */
    { 0x20, "NXP JCOP31 or JCOP41",       0, { 0 }, NULL },
    { 0x28, "NXP JCOP41",                 0, { 0 }, NULL },
    { 0x98, "Gemplus MPCOS",              0, { 0 }, NULL },
    /* @note I'm not sure that Jewel can be detected using this modulation but I haven't Jewel tags to test. */
    { 0x98, "Innovision R&T Jewel",       0, { 0 }, NULL },
};

void
print_iso14443a_name(const nfc_iso14443a_info_t nai)
{
  const char *tag_name = NULL;
  char *additionnal_info = NULL;
  
  for (size_t i = 0; i < sizeof (iso14443a_tags) / sizeof (struct iso14443a_tag); i++) {
    if ( (nai.btSak == iso14443a_tags[i].SAK) ) {
      // printf("DBG: iso14443a_tags[i].ATS_length = %d , nai.szAtsLen = %d", iso14443a_tags[i].ATS_length, nai.szAtsLen);
      if ( iso14443a_tags[i].identication_fct != NULL ) {
        additionnal_info = iso14443a_tags[i].identication_fct(nai);
      }
      
      if( iso14443a_tags[i].ATS_length == 0 ) {
        tag_name = (iso14443a_tags[i].name);
        break;
      }
      
      if( iso14443a_tags[i].ATS_length == nai.szAtsLen ) {
        if ( memcmp( nai.abtAts, iso14443a_tags[i].ATS, iso14443a_tags[i].ATS_length ) == 0 ) {
          tag_name = (iso14443a_tags[i].name);
          break;
        }
      }
    }
  }
  
  if( tag_name != NULL ) {
    if( additionnal_info != NULL ) {
      printf("%s%s (UID=", tag_name, additionnal_info);
      free(additionnal_info);
    } else {
      printf("%s (UID=", tag_name);
    }
    print_hex (nai.abtUid, nai.szUidLen);
    printf (")\n");
  } else {
    printf ("Unknown ISO14443A tag type: ");
    printf ("ATQA (SENS_RES): ");
    print_hex (nai.abtAtqa, 2);
    printf (", UID (NFCID%c): ", (nai.abtUid[0] == 0x08 ? '3' : '1'));
    print_hex (nai.abtUid, nai.szUidLen);
    printf (", SAK (SEL_RES): ");
    print_hex (&nai.btSak, 1);
    if (nai.szAtsLen) {
      printf (", ATS (ATR): ");
      print_hex (nai.abtAts, nai.szAtsLen);
    }
    printf ("\n");
  }
}

int
main (int argc, const char *argv[])
{
  nfc_target_info_t nti;
  uint8_t device_count = 0;
  uint8_t device_tag_count = 0;	// per device
  uint8_t tag_count = 0;	// total

  nfc_device_desc_t *pnddDevices;
  size_t szDeviceFound;
  size_t szTargetFound;
  
  (void)(argc);
  (void)(argv);

  // Try to open the NFC device
  if (!(pnddDevices = malloc (MAX_DEVICE_COUNT * sizeof (*pnddDevices)))) {
    ERR ("%s", "malloc() failed\n");
    return EXIT_FAILURE;
  }

  nfc_list_devices (pnddDevices, MAX_DEVICE_COUNT, &szDeviceFound);

  if (szDeviceFound == 0) {
    ERR ("%s", "No device found.");
  }

  for (size_t i = 0; i < szDeviceFound; i++) {
    pnd = nfc_connect (&(pnddDevices[i]));
    nfc_target_info_t anti[MAX_TARGET_COUNT];
    
    device_count++;
    device_tag_count = 0;

    if (pnd == NULL) {
      ERR ("%s", "Unable to connect to NFC device.");
      return EXIT_FAILURE;
    }
    nfc_initiator_init (pnd);

    // Drop the field for a while
    nfc_configure (pnd, NDO_ACTIVATE_FIELD, false);

    // Let the reader only try once to find a tag
    nfc_configure (pnd, NDO_INFINITE_SELECT, false);

    // Configure the CRC and Parity settings
    nfc_configure (pnd, NDO_HANDLE_CRC, true);
    nfc_configure (pnd, NDO_HANDLE_PARITY, true);
    nfc_configure (pnd, NDO_AUTO_ISO14443_4, true);

    // Enable field so more power consuming cards can power themselves up
    nfc_configure (pnd, NDO_ACTIVATE_FIELD, true);

    printf ("device = %s\n", pnd->acName);
    
    if (nfc_initiator_list_passive_targets(pnd, NM_ISO14443A_106, anti, MAX_TARGET_COUNT, &szTargetFound )) {
      size_t n;
      for(n=0; n<szTargetFound; n++) {
        print_iso14443a_name (anti[n].nai);
      }
    }
    device_tag_count += szTargetFound;
    
    if (nfc_initiator_list_passive_targets(pnd, NM_ISO14443B_106, anti, MAX_TARGET_COUNT, &szTargetFound )) {
      size_t n;
      for(n=0; n<szTargetFound; n++) {
        printf ("  ISO14443B: ");
        printf ("ATQB: ");
        print_hex (nti.nbi.abtAtqb, 12);
        printf (", ID: ");
        print_hex (nti.nbi.abtId, 4);
        printf (", CID: %02x", nti.nbi.btCid);
        if (nti.nbi.szInfLen > 0) {
          printf (", INF: ");
          print_hex (nti.nbi.abtInf, nti.nbi.szInfLen);
        }
        printf (", PARAMS: %02x %02x %02x %02x", nti.nbi.btParam1, nti.nbi.btParam2, nti.nbi.btParam3,
                nti.nbi.btParam4);
        printf ("\n");
      }
    }
    device_tag_count += szTargetFound;
    
    printf ("%d tag(s) on device.\n", device_tag_count);
    tag_count += device_tag_count;

    // Disable field 
    nfc_configure (pnd, NDO_ACTIVATE_FIELD, false);

    nfc_disconnect (pnd);
  }
  if (device_count > 1) {
    printf ("Total: %d tag(s) on %d device(s).\n", tag_count, device_count);
  }

  return EXIT_SUCCESS;
}
