#include <stdio.h>

#include <nfc/nfc.h>

#include "nfc-access.h"

static nfc_device_t* device;

int nfc_get_uid(char* uid)
{
	nfc_target_info_t target;

	// Try to open the NFC initiator (reader)
	device = nfc_connect(NULL);

	if ( device == NULL )
	{
		fprintf(stderr, "NFC initiator (reader) not found\n" );
		return ( 1 );
	}
	nfc_initiator_init ( device );

	// Drop the field for a while
	nfc_configure ( device,NDO_ACTIVATE_FIELD,false );

	// Let the reader only try once to find a tag
	nfc_configure ( device,NDO_INFINITE_SELECT,false );

	// Configure the CRC and Parity settings
	nfc_configure ( device,NDO_HANDLE_CRC,true );
	nfc_configure ( device,NDO_HANDLE_PARITY,true );

	// Enable field so more power consuming cards can power themselves up
	nfc_configure ( device,NDO_ACTIVATE_FIELD,true );

	// Poll for a ISO14443A (MIFARE) tag
	if ( nfc_initiator_select_tag ( device,NM_ISO14443A_106,NULL,0,&target ) )
	{
/*
		printf ( "The following (NFC) ISO14443A tag was found:\n\n" );
		printf ( "    ATQA (SENS_RES): " ); print_hex ( ti.tia.abtAtqa,2 );
		printf ( "       UID (NFCID%c): ", ( ti.tia.abtUid[0]==0x08?'3':'1' ) ); print_hex ( ti.tia.abtUid,ti.tia.uiUidLen );
		printf ( "      SAK (SEL_RES): " ); print_hex ( &ti.tia.btSak,1 );
		if ( ti.tia.uiAtsLen )
		{
			printf ( "          ATS (ATR): " );
			print_hex ( ti.tia.abtAts,ti.tia.uiAtsLen );
		}
*/
		size_t pos;
		char *p_uid = uid;
		for (pos=0; pos < target.nai.szUidLen; pos++)
		{
			sprintf(p_uid, "%02x",target.nai.abtUid[pos]);
			p_uid += 2;
		}
	} else {
		fprintf(stderr, "No ISO14443A (MIFARE) tag found.");
		nfc_disconnect ( device );
		return 1;
	}


/*
	// Poll for a Felica tag
	if ( nfc_reader_select ( device,IM_FELICA_212,abtFelica,5,&ti ) || nfc_reader_select ( device,IM_FELICA_424,abtFelica,5,&ti ) )
	{
		printf ( "The following (NFC) Felica tag was found:\n\n" );
		printf ( "%18s","ID (NFCID2): " ); print_hex ( ti.tif.abtId,8 );
		printf ( "%18s","Parameter (PAD): " ); print_hex ( ti.tif.abtPad,8 );
	}
*/
/*
	// Poll for a ISO14443B tag
	if ( nfc_reader_select ( device,IM_ISO14443B_106,NULL,0,&ti ) )
	{
		// No test results yet
		printf ( "iso14443b\n" );
	}
*/
/*
	// Poll for a Jewel tag
	if ( nfc_reader_select ( device,IM_JEWEL_106,NULL,0,&ti ) )
	{
		// No test results yet
		printf ( "jewel\n" );
	}
*/

	nfc_disconnect ( device );

//        complet=strdup(user); /*                         + user  */
//	strcat(complet," ");  /*                    + un espace  */
//	strcat(complet, crypt(chaine,"BA")); /*   + chaine code  */

//  complet[sizeof (complet) - 1] = 0; /* little check */
//   sprintf (complet, "%s %s", user, crypt(chaine,"BA"));
//      if (complet[sizeof (complet) - 1] != 0) {
//	      printf("\nInternal error!\n");
//      }
	return 0;
}

