/**
 * Public platform independent Near Field Communication (NFC) library
 * 
 * Copyright (C) 2009, François Kooman
 * Copyright (C) 2009, Romuald Conty
 * 
 * This file is based on nfc-mfultool.c (C) 2009 Roel Verdult.
 * 
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 * 
 * 
 * @file ndef-decode.c
 * @brief Small tool to decode NDEF message.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <string.h>
#include <ctype.h>

#include <locale.h>

#define DEBUG 1

typedef unsigned char byte_t;

/**
 * Extracts the NDEF message from a MIFARE Ultralight tag.
 * 
 * @param IN  mtD       the pointer to the mifareul_tag struct
 * @param OUT ndef_msg  returned NDEF message
 * @param OUT tlv_len   length of the returned NDEF message
 * @return              whether or not the ndef_msg was successfully
 *                      filled
 */
bool get_ndef_message_from_dump(byte_t* dump, byte_t * ndef_msg)
{
  int tlv_len = 0;
  // First byte is TLV type
  // We are looking for  "NDEF Message TLV" (0x03)
  if (dump[0] == 0x03) {
    tlv_len = dump[1];
    memcpy(ndef_msg, dump+2 , tlv_len);
    printf("NDEF message found in dump.\n");
  } else {
    printf("NDEF message TLV not found\n");
    return 0;
  }
  return 1;
}

/**
 * Convert TNF bits to Type (used for debug only). This is specified in 
 * section 3.2.6 "TNF (Type Name Format)" of "NFC Data Exchange Format 
 * (NDEF) Technical Specification".
 *
 * @param IN  b  the value of the last 3 bits of the NDEF record header
 * @return       the human readable type of the NDEF record
 */
char *type_name_format(int b)
{
    switch (b) {
    case 0x00:
	return "Empty";
    case 0x01:
	return "NFC Forum well-known type [NFC RTD]";
    case 0x02:
	return "Media-type as defined in RFC 2046 [RFC 2046]";
    case 0x03:
	return "Absolute URI as defined in RFC 3986 [RFC 3986]";
    case 0x04:
	return "NFC Forum external type [NFC RTD]";
    case 0x05:
	return "Unknown";
    case 0x06:
	return "Unchanged";
    case 0x07:
	return "Reserved";
    default:
	return "Invalid TNF byte!";
    }
}

/**
 * Convert type of URI to the actual URI prefix to be used in conjunction
 * with the URI stored in the NDEF record itself. This is specified in section 
 * 3.2.2 "URI Identifier Code" of "URI Record Type Definition Technical 
 * Specification".
 * 
 * @param IN  b  the code of the URI to convert to the actual prefix
 * @return       the URI prefix
 */
char *uri_identifier_code(int b)
{
    /*
     * Section 3.2.2 "URI Identifier Code" of "URI Record Type Definition
     * Technical Specification"
     */
    switch (b) {
    case 0x00:
	return "";
    case 0x01:
	return "http://www.";
    case 0x02:
	return "https://www.";
    case 0x03:
	return "http://";
    case 0x04:
	return "https://";
    case 0x05:
	return "tel:";
    case 0x06:
	return "mailto:";
    case 0x07:
	return "ftp://anonymous:anonymous@";
    case 0x08:
	return "ftp://ftp.";
    case 0x09:
	return "ftps://";
    case 0x0A:
	return "sftp://";
    case 0x0B:
	return "smb://";
    case 0x0C:
	return "nfs://";
    case 0x0D:
	return "ftp://";
    case 0x0E:
	return "dav://";
    case 0x0F:
	return "news:";
    case 0x10:
	return "telnet://";
    case 0x11:
	return "imap:";
    case 0x12:
	return "rtsp://";
    case 0x13:
	return "urn:";
    case 0x14:
	return "pop:";
    case 0x15:
	return "sip:";
    case 0x16:
	return "sips:";
    case 0x17:
	return "tftp:";
    case 0x18:
	return "btspp://";
    case 0x19:
	return "btl2cap://";
    case 0x1A:
	return "btgoep://";
    case 0x1B:
	return "tcpobex://";
    case 0x1C:
	return "irdaobex://";
    case 0x1D:
	return "file://";
    case 0x1E:
	return "urn:epc:id:";
    case 0x1F:
	return "urn:epc:tag:";
    case 0x20:
	return "urn:epc:pat:";
    case 0x21:
	return "urn:epc:raw:";
    case 0x22:
	return "urn:epc:";
    case 0x23:
	return "urn:nfc:";
    default:
	return "RFU";
    }
}

/**
 * Concatenates the prefix with the contents of the NDEF URI record.
 *
 * @param IN payload      The NDEF URI payload
 * @param IN payload_len  The length of the NDEF URI payload
 * @return                The full reconstructed URI 
 */
char *ndef_parse_uri(byte_t * payload, int payload_len)
{
//    if (DEBUG) print_hex(payload);
    char *prefix = uri_identifier_code(*payload);
    int prefix_len = strlen(prefix);
    char *url = malloc(prefix_len + payload_len);
    memcpy(url, prefix, prefix_len);
    memcpy(url + prefix_len, payload + 1, payload_len - 1);
    *(url + prefix_len + payload_len - 1) = 0x00;
    return url;
}

/**
 * Concatenates the prefix with the contents of the NDEF URI record.
 *
 * @param IN payload      The NDEF Text Record payload
 * @param IN payload_len  The length of the NDEF Text Record payload
 * @param OUT lang        The IANA language code lenght
 * @param OUT text        The text contained in NDEF Text Record
 * @return                Success or not.
 */
bool ndef_parse_text(byte_t * payload, int payload_len, char ** lang, char ** text)
{
//    if (DEBUG) print_hex(payload);
    bool utf16_format = ((payload[0] & 0x80) == 0x80);
    bool rfu = ((payload[0] & 0x40) == 0x40);
    if(rfu) {
      printf("ERROR: RFU should be set to zero.\n");
      return 0;
    }

    int lang_len = payload[0] & 0x3F;
    (*lang) = malloc(lang_len +1);
    memcpy(*lang, payload + 1, lang_len);
    (*lang)[lang_len] = '\0';

    const int text_len = payload_len - lang_len;

    if(DEBUG) printf("Format: %s, RFU=%d, IANA language code lenght=%d, text lenght=%d\n", utf16_format ? "UTF-16" : "UTF-8", rfu, lang_len, text_len);

    (*text) = malloc(text_len+1);
    memcpy(*text, payload + 1 + lang_len, text_len);
    (*text)[text_len] = L'\0';
    
    return 1;
}

/**
 * Parse the actual NDEF message and call specific handlers for dealing with
 * a particular type of NDEF message.
 * 
 * @param IN ndef_msg  The NDEF message
 * @return             whether or not the parsing succeeds
 */
bool parse_ndef_message(byte_t * ndef_msg)
{
    /* analyze the header field */
    int offset = 0;
    bool me;
    do {
	bool mb = (*(ndef_msg + offset) & 0x80) == 0x80;	/* Message Begin */
	me = (*(ndef_msg + offset) & 0x40) == 0x40;	/* Message End */
	bool cf = (*(ndef_msg + offset) & 0x20) == 0x20;	/* Chunk Flag */
	bool sr = (*(ndef_msg + offset) & 0x10) == 0x10;	/* Short Record */
	bool il = (*(ndef_msg + offset) & 0x08) == 0x08;	/* ID Length Present */
	char *typeNameFormat =
	    type_name_format(*(ndef_msg + offset) & 0x07);
	offset++;

	if (DEBUG) {
	    printf("MB=%d ME=%d CF=%d SR=%d IL=%d TNF=%s\n", mb, me, cf,
		   sr, il, typeNameFormat);
	}
	if (cf) {
	    printf("chunk flag not supported yet\n");
	    return 0;
	}

	int typeLength = *(ndef_msg + offset);
	offset++;

	int payloadLength;
	if (sr) {
	    payloadLength = *(ndef_msg + offset);
	    payloadLength = (payloadLength < 0) ? payloadLength + 256
		: payloadLength;
	    offset++;
	} else {
	    //FIXME payloadLength = Utils.byteArrayToInt(data, offset);
	    offset += 4;
	}

	int idLength = 0;
	if (il) {
	    idLength = *(ndef_msg + offset);
	    offset++;
	}
	char *type = malloc(typeLength);
	memcpy(type, ndef_msg + offset, typeLength);

	offset += typeLength;

	byte_t *id;
	if (il) {
	    id = malloc(idLength);
	    memcpy(id, ndef_msg + offset, idLength);
	    offset += idLength;
	}
	if (DEBUG)
	    printf("typeLength=%d payloadLength=%d idLength=%d type=%s\n",
		   typeLength, payloadLength, idLength, type);

	byte_t *payload = malloc(payloadLength);

	memcpy(payload, ndef_msg + offset, payloadLength);
	offset += payloadLength;

	if (strncmp("U", type, typeLength) == 0) {
	  /* handle URI case */
	  char *uri = ndef_parse_uri(payload, payloadLength);
	  printf("URI=%s\n", uri);
	} else if(strncmp("Sp", type, typeLength) == 0) {
          printf("Smart Poster - Begin\n");
          parse_ndef_message(payload);
          printf("Smart Poster - End\n");
        } else if(strncmp("T", type, typeLength) == 0) {
          char *lang;
          char *text;
          if(ndef_parse_text(payload, payloadLength, &lang, &text)) {
            printf("Text='%s' (Language=%s)\n", text, lang);
          } else {
            printf("Unable to parse one Text Record.\n");
          }
        } else {
          printf("unsupported NDEF record type: \"%s\"\n", type);
// 	    return 0;
	}
    } while (!me);		/* as long as this is not the last record */
}

#define BUFFER_SIZE 4096
int main(int argc, const char *argv[])
{
	FILE* dumpfile = NULL;
	byte_t dump[BUFFER_SIZE];
	memset(&dump, 0x00, sizeof(dump));

	dumpfile = fopen(argv[1],"rb");
	if (dumpfile == NULL)
	{
		printf("Could not open file: %s\n",argv[1]);
		return 1;
	}
	if (fread(&dump,1,sizeof(dump),dumpfile) != sizeof(dump))
	{
		printf("Could not read file: %s\n",argv[1]);
		fclose(dumpfile);
		return 1;
	}
	fclose(dumpfile);

	byte_t *ndef_msg = malloc(BUFFER_SIZE);
	if(get_ndef_message_from_dump(dump, ndef_msg))
		parse_ndef_message(ndef_msg);
	else return 1;

	return 0;
}
