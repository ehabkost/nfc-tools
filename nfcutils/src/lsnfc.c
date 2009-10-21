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

#include <libnfc.h>

static dev_info* pdi;
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
  tag_info ti;
  uint8_t tag_count = 0;

  // Try to open the NFC device
  pdi = nfc_connect(NULL);

  // If specific device is wanted, i.e. an ARYGON device on /dev/ttyUSB0
  /*
  nfc_device_desc_t ndd;
  ndd.pcDriver = "ARYGON";
  ndd.pcPort = "/dev/ttyUSB0";
  ndd.uiSpeed = 115200;

  pdi = nfc_connect(&ndd);
  */

  if (pdi == INVALID_DEVICE_INFO)
  {
    ERR("Unable to connect to NFC device.");
    return 1;
  }
  nfc_initiator_init(pdi);

  // Drop the field for a while
  nfc_configure(pdi,DCO_ACTIVATE_FIELD,false);

  // Let the reader only try once to find a tag
  nfc_configure(pdi,DCO_INFINITE_SELECT,false);

  // Configure the CRC and Parity settings
  nfc_configure(pdi,DCO_HANDLE_CRC,true);
  nfc_configure(pdi,DCO_HANDLE_PARITY,true);

  // Enable field so more power consuming cards can power themselves up
  nfc_configure(pdi,DCO_ACTIVATE_FIELD,true);

  bool no_more_tag = false;
  printf("device = %s\n",pdi->acName);
  do {
    if (nfc_initiator_select_tag(pdi,IM_ISO14443A_106,NULL,0,&ti))  // Poll for a ISO14443A (MIFARE) tag
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

      if ((ti.tia.abtAtqa[0] == 0x00) && (ti.tia.abtAtqa[1] == 0x04) && (ti.tia.btSak == 0x09)) {
        printf("NXP MIFARE Mini (UID="); print_hex(ti.tia.abtUid,ti.tia.szUidLen); printf(")\n");
      } else if ((ti.tia.abtAtqa[0] == 0x00) && (ti.tia.abtAtqa[1] == 0x04) && (ti.tia.btSak == 0x08)) {
        printf("NXP MIFARE Classic 1K (UID="); print_hex(ti.tia.abtUid,ti.tia.szUidLen); printf(")\n");
      } else if ((ti.tia.abtAtqa[0] == 0x00) && (ti.tia.abtAtqa[1] == 0x02) && (ti.tia.btSak == 0x18)) {
        printf("NXP MIFARE Classic 4K (UID="); print_hex(ti.tia.abtUid,ti.tia.szUidLen); printf(")\n");
      } else if ((ti.tia.abtAtqa[0] == 0x00) && (ti.tia.abtAtqa[1] == 0x02) && (ti.tia.btSak == 0x38)) {
        printf("Nokia MIFARE Classic 4K - emulated - (UID="); print_hex(ti.tia.abtUid,ti.tia.szUidLen); printf(")\n");
      } else if ((ti.tia.abtAtqa[0] == 0x00) && (ti.tia.abtAtqa[1] == 0x44) && (ti.tia.btSak == 0x00)) {
        printf("NXP MIFARE Ultralight (UID="); print_hex(ti.tia.abtUid,ti.tia.szUidLen); printf(")\n");
      } else if ((ti.tia.abtAtqa[0] == 0x03) && (ti.tia.abtAtqa[1] == 0x44) && (ti.tia.btSak == 0x20)) {
        printf("NXP MIFARE DESFire (UID="); print_hex(ti.tia.abtUid,ti.tia.szUidLen); printf(")\n");
      } else if ((ti.tia.abtAtqa[0] == 0x03) && (ti.tia.abtAtqa[1] == 0x04) && (ti.tia.btSak == 0x28)) {
        printf("NXP JCOP31 (UID="); print_hex(ti.tia.abtUid,ti.tia.szUidLen); printf(")\n");
      } else if ((ti.tia.abtAtqa[0] == 0x00) && (ti.tia.abtAtqa[1] == 0x48) && (ti.tia.btSak == 0x20)) {
        /* @todo handle ATS to be able to know which one is it. */
        printf("NXP JCOP31 or JCOP41 (UID="); print_hex(ti.tia.abtUid,ti.tia.szUidLen); printf(")\n");
      } else if ((ti.tia.abtAtqa[0] == 0x00) && (ti.tia.abtAtqa[1] == 0x04) && (ti.tia.btSak == 0x28)) {
        printf("NXP JCOP41 (UID="); print_hex(ti.tia.abtUid,ti.tia.szUidLen); printf(")\n");
      } else if ((ti.tia.abtAtqa[0] == 0x00) && (ti.tia.abtAtqa[1] == 0x04) && (ti.tia.btSak == 0x88)) {
        printf("Infineon MIFARE Classic 1K (UID="); print_hex(ti.tia.abtUid,ti.tia.szUidLen); printf(")\n");
      } else if ((ti.tia.abtAtqa[0] == 0x00) && (ti.tia.abtAtqa[1] == 0x02) && (ti.tia.btSak == 0x98)) {
        printf("Gemplus MPCOS (UID="); print_hex(ti.tia.abtUid,ti.tia.szUidLen); printf(")\n");
      } else if ((ti.tia.abtAtqa[0] == 0x0C) && (ti.tia.abtAtqa[1] == 0x00)) {
        /* @note I'm not sure that Jewel can be detected using this modultation and I haven't Jewel tags to test. */
        printf("Innovision R&T Jewel (UID="); print_hex(ti.tia.abtUid,ti.tia.szUidLen); printf(")\n");
      } else {
        printf("Unknown tag type: ");
        printf("ATQA (SENS_RES): "); print_hex(ti.tia.abtAtqa,2);
        printf(", UID (NFCID%c): ",(ti.tia.abtUid[0]==0x08?'3':'1')); print_hex(ti.tia.abtUid,ti.tia.szUidLen);
        printf(", SAK (SEL_RES): "); print_hex(&ti.tia.btSak,1);
        if (ti.tia.szAtsLen)
        {
          printf(", ATS (ATR): ");
          print_hex(ti.tia.abtAts,ti.tia.szAtsLen);
        }
        printf("\n");
      }
      nfc_initiator_deselect_tag(pdi);
      tag_count++;
    } else if (nfc_initiator_select_tag(pdi,IM_FELICA_212,abtFelica,5,&ti) || nfc_initiator_select_tag(pdi,IM_FELICA_424,abtFelica,5,&ti))  // Poll for a Felica tag
    {
      printf("  Felica: ");
      printf("ID (NFCID2): "); print_hex(ti.tif.abtId,8);
      printf(", Parameter (PAD): "); print_hex(ti.tif.abtPad,8);
      printf("\n");
      nfc_initiator_deselect_tag(pdi);
      tag_count++;
    } else if (nfc_initiator_select_tag(pdi,IM_ISO14443B_106,(byte_t*)"\x00",1,&ti))  // Poll for a ISO14443B tag
    {
      printf("  ISO14443B: ");
      printf("ATQB: "); print_hex(ti.tib.abtAtqb,12);
      printf(", ID: "); print_hex(ti.tib.abtId,4);
      printf(", CID: %02x\n",ti.tib.btCid);
      if (ti.tib.szInfLen>0)
      {
        printf(", INF: "); print_hex(ti.tib.abtInf,ti.tib.szInfLen);
      }
      printf(", PARAMS: %02x %02x %02x %02x\n",ti.tib.btParam1,ti.tib.btParam2,ti.tib.btParam3,ti.tib.btParam4);
      printf("\n");
      nfc_initiator_deselect_tag(pdi);
      tag_count++;
    } else if (nfc_initiator_select_tag(pdi,IM_JEWEL_106,NULL,0,&ti))  // Poll for a Jewel tag
    {
      printf("  Jewel: No test results yet");
      nfc_initiator_deselect_tag(pdi);
      tag_count++;
    } else {
      no_more_tag = true;
    }
  } while (no_more_tag != true);
  printf("%d tag(s) have been found.\n", tag_count);

  // Disable field 
  nfc_configure(pdi,DCO_ACTIVATE_FIELD,false);

  nfc_disconnect(pdi);
  return 1;
}
