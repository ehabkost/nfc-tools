 /**
 * NFC utils - lsnfc
 *
 * Copyright (C) 2009, Romuald Conty
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

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include <string.h>

#include <nfc.h>

static nfc_device_t* pnd;
static byte_t abtFelica[5] = { 0x00, 0xff, 0xff, 0x00, 0x00 };

#define ERR(x, ...) printf("ERROR: " x "\n", ## __VA_ARGS__ )

void print_hex(byte_t* pbtData, size_t szDate)
{
  for(size_t i=0; i<szDate; i++) {
    printf("%02x",pbtData[i]);
  }
}

int main(int argc, const char* argv[])
{
  nfc_target_info_t nti;
  uint8_t tag_count = 0;

  // Try to open the NFC device
  pnd = nfc_connect(NULL);

  // If specific device is wanted, i.e. an ARYGON device on /dev/ttyUSB0
  /*
  nfc_device_desc_t ndd;
  ndd.pcDriver = "ARYGON";
  ndd.pcPort = "/dev/ttyUSB0";
  ndd.uiSpeed = 115200;

  pnd = nfc_connect(&ndd);
  */

  if (pnd == NULL)
  {
    ERR("Unable to connect to NFC device.");
    return 1;
  }
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

  bool no_more_tag = false;
  printf("device = %s\n",pnd->acName);
  do {
    if (nfc_initiator_select_tag(pnd,NM_ISO14443A_106,NULL,0,&nti))  // Poll for a ISO14443A (MIFARE) tag
    {
      printf("  ISO14443A: ");

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

      if ((nti.nai.abtAtqa[0] == 0x00) && (nti.nai.abtAtqa[1] == 0x04) && (nti.nai.btSak == 0x09)) {
        printf("NXP MIFARE Mini (UID="); print_hex(nti.nai.abtUid,nti.nai.szUidLen); printf(")\n");
      } else if ((nti.nai.abtAtqa[0] == 0x00) && (nti.nai.abtAtqa[1] == 0x04) && (nti.nai.btSak == 0x08)) {
        printf("NXP MIFARE Classic 1K (UID="); print_hex(nti.nai.abtUid,nti.nai.szUidLen); printf(")\n");
      } else if ((nti.nai.abtAtqa[0] == 0x00) && (nti.nai.abtAtqa[1] == 0x02) && (nti.nai.btSak == 0x18)) {
        printf("NXP MIFARE Classic 4K (UID="); print_hex(nti.nai.abtUid,nti.nai.szUidLen); printf(")\n");
      } else if ((nti.nai.abtAtqa[0] == 0x00) && (nti.nai.abtAtqa[1] == 0x02) && (nti.nai.btSak == 0x38)) {
        printf("Nokia MIFARE Classic 4K - emulated - (UID="); print_hex(nti.nai.abtUid,nti.nai.szUidLen); printf(")\n");
      } else if ((nti.nai.abtAtqa[0] == 0x00) && (nti.nai.abtAtqa[1] == 0x44) && (nti.nai.btSak == 0x00)) {
        printf("NXP MIFARE Ultralight (UID="); print_hex(nti.nai.abtUid,nti.nai.szUidLen); printf(")\n");
      } else if ((nti.nai.abtAtqa[0] == 0x03) && (nti.nai.abtAtqa[1] == 0x44) && (nti.nai.btSak == 0x20)) {
        printf("NXP MIFARE DESFire (UID="); print_hex(nti.nai.abtUid,nti.nai.szUidLen); printf(")\n");
      } else if ((nti.nai.abtAtqa[0] == 0x03) && (nti.nai.abtAtqa[1] == 0x04) && (nti.nai.btSak == 0x28)) {
        printf("NXP JCOP31 (UID="); print_hex(nti.nai.abtUid,nti.nai.szUidLen); printf(")\n");
      } else if ((nti.nai.abtAtqa[0] == 0x00) && (nti.nai.abtAtqa[1] == 0x48) && (nti.nai.btSak == 0x20)) {
        /* @todo handle ATS to be able to know which one is it. */
        printf("NXP JCOP31 or JCOP41 (UID="); print_hex(nti.nai.abtUid,nti.nai.szUidLen); printf(")\n");
      } else if ((nti.nai.abtAtqa[0] == 0x00) && (nti.nai.abtAtqa[1] == 0x04) && (nti.nai.btSak == 0x28)) {
        printf("NXP JCOP41 (UID="); print_hex(nti.nai.abtUid,nti.nai.szUidLen); printf(")\n");
      } else if ((nti.nai.abtAtqa[0] == 0x00) && (nti.nai.abtAtqa[1] == 0x04) && (nti.nai.btSak == 0x88)) {
        printf("Infineon MIFARE Classic 1K (UID="); print_hex(nti.nai.abtUid,nti.nai.szUidLen); printf(")\n");
      } else if ((nti.nai.abtAtqa[0] == 0x00) && (nti.nai.abtAtqa[1] == 0x02) && (nti.nai.btSak == 0x98)) {
        printf("Gemplus MPCOS (UID="); print_hex(nti.nai.abtUid,nti.nai.szUidLen); printf(")\n");
      } else if ((nti.nai.abtAtqa[0] == 0x0C) && (nti.nai.abtAtqa[1] == 0x00)) {
        /* @note I'm not sure that Jewel can be detected using this modultation and I haven't Jewel tags to test. */
        printf("Innovision R&T Jewel (UID="); print_hex(nti.nai.abtUid,nti.nai.szUidLen); printf(")\n");
      } else {
        printf("Unknown tag type: ");
        printf("ATQA (SENS_RES): "); print_hex(nti.nai.abtAtqa,2);
        printf(", UID (NFCID%c): ",(nti.nai.abtUid[0]==0x08?'3':'1')); print_hex(nti.nai.abtUid,nti.nai.szUidLen);
        printf(", SAK (SEL_RES): "); print_hex(&nti.nai.btSak,1);
        if (nti.nai.szAtsLen)
        {
          printf(", ATS (ATR): ");
          print_hex(nti.nai.abtAts,nti.nai.szAtsLen);
        }
        printf("\n");
      }
      nfc_initiator_deselect_tag(pnd);
      tag_count++;
    } else if (nfc_initiator_select_tag(pnd,NM_FELICA_212,abtFelica,5,&nti) || nfc_initiator_select_tag(pnd,NM_FELICA_424,abtFelica,5,&nti))  // Poll for a Felica tag
    {
      printf("  Felica: ");
      printf("ID (NFCID2): "); print_hex(nti.nfi.abtId,8);
      printf(", Parameter (PAD): "); print_hex(nti.nfi.abtPad,8);
      printf("\n");
      nfc_initiator_deselect_tag(pnd);
      tag_count++;
    } else if (nfc_initiator_select_tag(pnd,NM_ISO14443B_106,(byte_t*)"\x00",1,&nti))  // Poll for a ISO14443B tag
    {
      printf("  ISO14443B: ");
      printf("ATQB: "); print_hex(nti.nbi.abtAtqb,12);
      printf(", ID: "); print_hex(nti.nbi.abtId,4);
      printf(", CID: %02x\n",nti.nbi.btCid);
      if (nti.nbi.szInfLen>0)
      {
        printf(", INF: "); print_hex(nti.nbi.abtInf,nti.nbi.szInfLen);
      }
      printf(", PARAMS: %02x %02x %02x %02x\n",nti.nbi.btParam1,nti.nbi.btParam2,nti.nbi.btParam3,nti.nbi.btParam4);
      printf("\n");
      nfc_initiator_deselect_tag(pnd);
      tag_count++;
    } else if (nfc_initiator_select_tag(pnd,NM_JEWEL_106,NULL,0,&nti))  // Poll for a Jewel tag
    {
      printf("  Jewel: No test results yet");
      nfc_initiator_deselect_tag(pnd);
      tag_count++;
    } else {
      no_more_tag = true;
    }
  } while (no_more_tag != true);
  printf("%d tag(s) have been found.\n", tag_count);

  // Disable field 
  nfc_configure(pnd,NDO_ACTIVATE_FIELD,false);

  nfc_disconnect(pnd);
  return 0;
}
