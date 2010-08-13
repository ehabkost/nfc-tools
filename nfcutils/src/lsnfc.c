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

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include <string.h>

#include <nfc/nfc.h>

static nfc_device_t *pnd;
static byte_t abtFelica[5] = { 0x00, 0xff, 0xff, 0x00, 0x00 };

#define ERR(x, ...) printf("ERROR: " x "\n", __VA_ARGS__ )

#define MAX_DEVICE_COUNT	16
#define MAX_ATS_LENGTH		32

struct iso14443a_tag {
    uint8_t SAK;
    const char *name;
    size_t ATS_length;
    uint8_t ATS[MAX_ATS_LENGTH];
};

/*
Theses values comes from http://www.libnfc.org/documentation/hardware/tags/iso14443

Manufacturer	Product			ATQA	SAK	ATS (called ATR for contact smartcards) 

NXP		MIFARE Mini		00 04 	09 	
		MIFARE Classic 1K 	00 04 	08 	
		MIFARE Classic 4K 	00 02 	18 	
		MIFARE Ultralight 	00 44 	00 	
		MIFARE DESFire		03 44 	20 	06 75 77 81 02 80
		MIFARE DESFire EV1 	03 44 	20 	06 75 77 81 02 80
		JCOP31			03 04 	28 	38 77 b1 4a 43 4f 50 33 31
		JCOP31 v2.4.1		00 48 	20 	78 77 b1 02 4a 43 4f 50 76 32 34 31
		JCOP41 v2.2		00 48 	20 	38 33 b1 4a 43 4f 50 34 31 56 32 32
		JCOP41 v2.3.1		00 04 	28 	38 33 b1 4a 43 4f 50 34 31 56 32 33 31
Infineon 	MIFARE Classic 1K 	00 04 	88 	
Gemplus 	MPCOS 			00 02 	98
Innovision R&T 	Jewel 			0C 00
*/

struct iso14443a_tag iso14443a_tags[] = {
    { 0x00, "NXP MIFARE UltraLight",      0, { 0 } },
    { 0x00, "NXP MIFARE UltraLight C",    0, { 0 } },
    { 0x09, "NXP MIFARE Mini",            0, { 0 } },
    { 0x08, "NXP MIFARE Classic 1k",      0, { 0 } },
    { 0x18, "NXP MIFARE Classic 4k",      0, { 0 } },

    { 0x08, "NXP MIFARE Plus 1k",         0, { 0 } },
    { 0x18, "NXP MIFARE Plus 4k",         0, { 0 } },
    { 0x10, "NXP MIFARE Plus 1k",         0, { 0 } },
    { 0x11, "NXP MIFARE Plus 4k",         0, { 0 } },
    { 0x20, "NXP MIFARE Plus 1k",         0, { 0 } },
    { 0x20, "NXP MIFARE Plus 4k",         0, { 0 } },
    { 0x20, "NXP MIFARE Plus 1k/4k",     11, { 0x75, 0x77, 0x80, 0x02, 0xc1, 0x05, 0x2f, 0x2f, 0x01, 0xbc, 0xd6} },
    { 0x20, "NXP MIFARE Plus 2k/4k",     11, { 0x75, 0x77, 0x80, 0x02, 0xc1, 0x05, 0x2f, 0x2f, 0x01, 0xbc, 0xd6 } },

    { 0x88, "Infineon MIFARE Classic 1k", 0, { 0 } },
    { 0x38, "Nokia MIFARE Classic 4k (Emulated)", 0, { 0 } },
    { 0x20, "NXP MIFARE DESFire",         5, { 0x75, 0x77, 0x81, 0x02, 0x80 } },
    { 0x28, "NXP JCOP31",                 0, { 0 } },
    /* @todo handle ATS to be able to know which one is it. */
    { 0x20, "NXP JCOP31 or JCOP41",       0, { 0 } },
    { 0x28, "NXP JCOP41",                 0, { 0 } },
    { 0x98, "Gemplus MPCOS",              0, { 0 } },
    /* @note I'm not sure that Jewel can be detected using this modulation but I haven't Jewel tags to test. */
    { 0x98, "Innovision R&T Jewel",       0, { 0 } },
};

void
print_hex (byte_t * pbtData, size_t szDate)
{
  for (size_t i = 0; i < szDate; i++) {
    printf ("%02x", pbtData[i]);
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
  size_t szFound;

  (void)(argc, argv);

  // Try to open the NFC device
  if (!(pnddDevices = malloc (MAX_DEVICE_COUNT * sizeof (*pnddDevices)))) {
    ERR ("%s", "malloc() failed\n");
    return EXIT_FAILURE;
  }

  nfc_list_devices (pnddDevices, MAX_DEVICE_COUNT, &szFound);

  if (szFound == 0) {
    ERR ("%s", "No device found.");
  }

  for (size_t i = 0; i < szFound; i++) {
    pnd = nfc_connect (&(pnddDevices[i]));

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

    // Enable field so more power consuming cards can power themselves up
    nfc_configure (pnd, NDO_ACTIVATE_FIELD, true);

    bool no_more_tag = false;
    printf ("device = %s\n", pnd->acName);
    do {
      if (nfc_initiator_select_passive_target (pnd, NM_ISO14443A_106, NULL, 0, &nti)) {
        printf ("  ISO14443A: ");
        const char *tag_name = NULL;

        for (size_t i = 0; i < sizeof (iso14443a_tags) / sizeof (struct iso14443a_tag); i++) {
            if ( (nti.nai.btSak == iso14443a_tags[i].SAK) ) {
                // printf("DBG: iso14443a_tags[i].ATS_length = %d , nti.nai.szAtsLen = %d", iso14443a_tags[i].ATS_length, nti.nai.szAtsLen);
                if( iso14443a_tags[i].ATS_length == 0 ) {
                    tag_name = (iso14443a_tags[i].name);
                    break;
                }

                if( iso14443a_tags[i].ATS_length == nti.nai.szAtsLen ) {
                    if ( memcmp( nti.nai.abtAts, iso14443a_tags[i].ATS, iso14443a_tags[i].ATS_length ) == 0 ) {
                        tag_name = (iso14443a_tags[i].name);
                        break;
                    }
                }
            }
        }
        if( tag_name != NULL ) {
            printf("%s (UID=", tag_name);
            print_hex (nti.nai.abtUid, nti.nai.szUidLen);
            printf (")\n");
        } else {
          printf ("Unknown ISO14443A tag type: ");
          printf ("ATQA (SENS_RES): ");
          print_hex (nti.nai.abtAtqa, 2);
          printf (", UID (NFCID%c): ", (nti.nai.abtUid[0] == 0x08 ? '3' : '1'));
          print_hex (nti.nai.abtUid, nti.nai.szUidLen);
          printf (", SAK (SEL_RES): ");
          print_hex (&nti.nai.btSak, 1);
          if (nti.nai.szAtsLen) {
            printf (", ATS (ATR): ");
            print_hex (nti.nai.abtAts, nti.nai.szAtsLen);
          }
          printf ("\n");
        }
        nfc_initiator_deselect_target (pnd);
        device_tag_count++;
      } else if (nfc_initiator_select_passive_target (pnd, NM_FELICA_212, abtFelica, 5, &nti)
                 || nfc_initiator_select_passive_target (pnd, NM_FELICA_424, abtFelica, 5, &nti)) {
        printf ("  Felica: ");
        printf ("ID (NFCID2): ");
        print_hex (nti.nfi.abtId, 8);
        printf (", Parameter (PAD): ");
        print_hex (nti.nfi.abtPad, 8);
        printf ("\n");
        nfc_initiator_deselect_target (pnd);
        device_tag_count++;
      } else if (nfc_initiator_select_passive_target (pnd, NM_ISO14443B_106, (byte_t *) "\x00", 1, &nti)) {
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
        nfc_initiator_deselect_target (pnd);
        device_tag_count++;
      } else if (nfc_initiator_select_passive_target (pnd, NM_JEWEL_106, NULL, 0, &nti)) {
        printf ("  Jewel: No test results yet");
        nfc_initiator_deselect_target (pnd);
        device_tag_count++;
      } else {
        no_more_tag = true;
      }
    } while (no_more_tag != true);
    printf ("%d tag(s) on device.\n\n", device_tag_count);
    tag_count += device_tag_count;

    // Disable field 
    nfc_configure (pnd, NDO_ACTIVATE_FIELD, false);

    nfc_disconnect (pnd);
  }
  if (device_count > 1)
      printf ("Total: %d tag(s) on %d device(s).\n", tag_count, device_count);
  return EXIT_SUCCESS;
}
