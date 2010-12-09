#include "NfcTarget.h"

#include <QDebug>
#include <ndef/ndefrecordtype.h>

#include <stdlib.h>
#include <nfc/nfc.h>
#include "NFCd.h"
#include "NfcForumTag.h"
#include "nfcforum_type4_tag.h"

/*
 * This implementation was written based on information provided by the
 * following documents:
 *
 * NXP Type MF1K/4K Tag Operation
 * Storing NFC Forum data in Mifare Standard 1k/4k
 * Rev. 1.1 - 21 August 2007
 */

static const MifareClassicKey default_keys[] = {
  { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
  { 0xd3, 0xf7, 0xd3, 0xf7, 0xd3, 0xf7 },
  { 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5 },
  { 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5 },
  { 0x4d, 0x3a, 0x99, 0xc3, 0x51, 0xdd },
  { 0x1a, 0x98, 0x2c, 0x7e, 0x45, 0x9a },
  { 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff },
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
};

static const MifareClassicKey default_key_b = {
  0xd3, 0xf7, 0xd3, 0xf7, 0xd3, 0xf7
};

NfcTarget::NfcTarget (ISO14443bTag tag, nfc_device_t* device , QMutex* accessLock)
{
  _accessLock = accessLock;
  _tag14443b = tag;
  char* u = iso14443b_get_tag_uid ( _tag14443b );
  _uid = QString ( u );
  _uuid = QUuid::createUuid();
  free ( u );
  _name = iso14443b_get_tag_name ( _tag14443b );
  _type = ISO14443B;
  _device = device;
//   _nfcforumvalidstate = isNFCForumValidTag();
//   checkAvailableContent();
  checkNfcForumTag();
}

NfcTarget::NfcTarget ( MifareTag tag, nfc_device_t* device, QMutex* accessLock )
{
  _accessLock = accessLock;
  _tag = tag;
  char* u = freefare_get_tag_uid ( _tag );
  _uid = QString ( u );
  _uuid = QUuid::createUuid();
  free ( u );
  _name = freefare_get_tag_friendly_name ( _tag );
  _type = MIFARE;
  _device = device;
  // checkAvailableContent();
  checkNfcForumTag();
}

NfcTarget::~NfcTarget()
{
  if ( _type == ISO14443B ) {
    iso14443b_free_tag ( _tag14443b );
  } else {
    freefare_free_tag ( _tag );
  }
}

const QString NfcTarget::getName()
{
  return _name;
}

const tag_type NfcTarget::getType()
{
  return _type;
}

const QString NfcTarget::getUid()
{
  return _uid;
}

const QUuid NfcTarget::getUuid()
{
  return _uuid;
}

const QString NfcTarget::getPath()
{
  return _path;
}

void NfcTarget::setPath ( QString path )
{
  _path = path;
}

#define MIFARE_CONTENT_BUFFER_SIZE 4096
#define ISO14443B_CONTENT_BUFFER_SIZE 4096
bool
NfcTarget::isNFCForumValidTag()
{
  switch(_state) {
    case NfcTarget::NFCFORUM_INITIALISED:
    case NfcTarget::NFCFORUM_INVALID:
    case NfcTarget::NFCFORUM_READONLY:
    case NfcTarget::NFCFORUM_READWRITE:
      return true;
    default:
      return false;
  }
}

void
NfcTarget::checkNfcForumTag()
{
  _accessLock->lock();
  int resConnect = -1;
  NfcForumTag* nfcForumTag;

  switch(_type) {
    case ISO14443B:
      resConnect = iso14443b_connect ( _tag14443b );
      if ( resConnect == 0 ) {
        _state = iso7816_4_checkNFCForumTag();
        if ( iso14443b_disconnect ( _tag14443b ) == 0 ) {};
      } else {
        _state = NfcTarget::UNKNOWN;
      }
      if ((_state == NFCFORUM_INITIALISED)||(_state == NFCFORUM_READWRITE)){
        nfcForumTag = new NfcForumTag(this);
      }
      break;
    case MIFARE:
      enum mifare_tag_type type = freefare_get_tag_type ( _tag );
      switch ( type ) {
        case CLASSIC_1K:
        case CLASSIC_4K:
          resConnect = mifare_classic_connect ( _tag );
          if ( 0 == resConnect ) {
            _state = mifare_classic_checkNFCForumTag();
            if ( mifare_classic_disconnect ( _tag ) == 0 ) {};
          } else {
            _state = NfcTarget::UNKNOWN;
          }
          if ((_state == NFCFORUM_INITIALISED)||(_state == NFCFORUM_READWRITE)){
            nfcForumTag = new NfcForumTag(this);
          }
          break;
        case ULTRALIGHT:
          resConnect =  mifare_ultralight_connect ( _tag );
          if ( 0 == resConnect ) {
            _state = mifare_ultralight_checkNFCForumTag();
            if ( mifare_ultralight_disconnect ( _tag ) == 0 ) {};
          } else {
            _state = NfcTarget::UNKNOWN;
          }
          if ((_state == NFCFORUM_INITIALISED)||(_state == NFCFORUM_READWRITE)){
            nfcForumTag = new NfcForumTag(this);
          }
          break;
        case DESFIRE:
          resConnect = mifare_desfire_connect(_tag );
          if ( 0 == resConnect ) {
            _state = iso7816_4_checkNFCForumTag();
            if ( mifare_desfire_disconnect ( _tag ) == 0 ) {};
          } else {
            _state = NfcTarget::UNKNOWN;
          }
          if ((_state == NFCFORUM_INITIALISED)||(_state == NFCFORUM_READWRITE)){
            nfcForumTag = new NfcForumTag(this);
          }
          break;
      }
      break;
  }
  _accessLock->unlock();
}

NfcTarget::State
NfcTarget::iso7816_4_checkNFCForumTag()
{ /*
  * NFC Forum type 4 tag
  */
  byte_t data[4096];
  size_t szData;
  uint16_t ui16NLen;
  uint16_t ui16MaxNDEFLen;
  uint16_t ui16CCLen;
  QByteArray ndefContent;

  //Select the NDEF Tag Application
  if ( ! nfcforum_type4_ndef_tag_application_select ( _device ) ) return NFCFORUM_INVALID;
  //Select the Capability Container (CC)
  const byte_t CCid[] = {0xE1, 0x03};
  if ( ! nfcforum_type4_select ( _device, CCid ) ) return NFCFORUM_INVALID;

  // Check the CC File
  if ( ! nfcforum_type4_read ( _device, 0x0000, 0x0f, data, &szData ) ) return NFCFORUM_INVALID;

  if ( ( data[7] == 0x04 ) && ( data[8] == 0x06 ) ) {
    byte_t  fileID[2];
    byte_t  readAccess[1];
    byte_t  writeAccess[1];
    ui16CCLen = 0x0000;
    ui16CCLen = ( data[0] << 8 ) | data[1];
    ui16MaxNDEFLen = 0x0000;
    ui16MaxNDEFLen = ( data[11] << 8 ) | data[12];
    memcpy ( fileID, data + 9, sizeof ( fileID ) );
    memcpy ( readAccess, data + 13, sizeof ( readAccess ) );
    memcpy ( writeAccess, data + 14, sizeof ( writeAccess ) );

    // The NDEF file, indicated by the file Identifier of the NDEF File Control TLV
    // of the CC file at the offset 0007h, is open for read and write access

    if ( ! nfcforum_type4_select ( _device, fileID ) ) return NFCFORUM_INVALID;

    if ( ! nfcforum_type4_read ( _device, 0x0000, 0x02, data, &szData ) ) return NFCFORUM_INVALID;
    ui16NLen = 0x0000;
    ui16NLen = ( data[0] << 8 ) | data[1];

    /*
    * Initialised state (read and write access and NLEN is equal to 0000h)
    */
    if ( ( readAccess[0] == 0x00 ) && ( writeAccess[0] == 0x00 ) && ( ui16NLen == 0x0000 ) ) {
      return NFCFORUM_INITIALISED;
    }
    /*
    * Read/Write State (read and write access and NLEN is not equal to 0000h)
    */
    else if ( ( readAccess[0] == 0x00 ) && ( writeAccess[0] == 0x00 ) && ( ui16NLen != 0x0000 ) ) {
      if ((ui16CCLen >= 0x000F) && ((ui16NLen <= (ui16MaxNDEFLen-2)))) {
        if(! nfcforum_type4_read(_device, 0x0002, data[1], data,&szData)) return NFCFORUM_INVALID;
        char buffer [ISO14443B_CONTENT_BUFFER_SIZE];
        memcpy(buffer,data,ISO14443B_CONTENT_BUFFER_SIZE);
        ndefContent.append(buffer);
        if ( !ndefContent.isEmpty() ) {
           NFCd::getNfcForumManager()->newAvailableNdefContent(ndefContent);
        }
      }
      return NFCFORUM_READWRITE;
    }
    /*
    * Read only State (read access, no write access and NLEN is not equal to 0000h)
    */
    else if ( ( readAccess[0] == 0x00 ) && ( writeAccess[0] == 0xff ) && ( ui16NLen != 0x0000 ) ) {
      if ((ui16CCLen >= 0x000F) && ((ui16NLen <= (ui16MaxNDEFLen-2))))
      {
        if(! nfcforum_type4_read(_device, 0x0002, data[1], data,&szData)) return NFCFORUM_INVALID;
        char buffer [ISO14443B_CONTENT_BUFFER_SIZE];
        memcpy(buffer,data,ISO14443B_CONTENT_BUFFER_SIZE);
        ndefContent.append(buffer);
        if ( !ndefContent.isEmpty() ) {
          NFCd::getNfcForumManager()->newAvailableNdefContent(ndefContent);
        }
      }
      return NFCFORUM_READONLY;
    }
  }
}

NfcTarget::State
NfcTarget::mifare_classic_checkNFCForumTag()
{
  // Authentication operation is performed with public key A (0xa0 0xa1 0xa2 0xa3 0xa4 0xa5)
  // for MAD sector to access Sector 0 i.e. the MAD sector.
  Mad mad = mad_read ( _tag );
  QByteArray ndefContent;

  if ( mad != NULL ) {
    uint8_t* buffer = ( uint8_t* ) malloc ( MIFARE_CONTENT_BUFFER_SIZE * sizeof ( uint8_t ) );
    ssize_t res = 0;
    // The General Purpose Byte (GPB) of each NFC Forum sector provides information about the
    // version number of the mapping model used to store the NFC Forum defined data into the
    // Mifare Standard 1k/4k (see section 6.1.1) and the write access of the NFC Forum sectors.
    MifareClassicSectorNumber *sectors = mifare_application_find (mad, mad_nfcforum_aid);
    if (!sectors)
      qDebug ( "Sector error" );

    MifareClassicBlockNumber last_block  = mifare_classic_sector_last_block (sectors[0]);
    MifareClassicBlock block;

    if (mifare_classic_authenticate (_tag, last_block, mifare_classic_nfcforum_public_key_a, MFC_KEY_A) < 0) {
      res = -1;
    }
    if (mifare_classic_read (_tag, last_block, &block) < 0) {
      res = -1;
    }
    uint8_t gpb = *(((uint8_t*)&block) + 9);
    free (sectors);

    ssize_t readed_bytes = mifare_application_read ( _tag, mad, mad_nfcforum_aid, buffer, MIFARE_CONTENT_BUFFER_SIZE, mifare_classic_nfcforum_public_key_a, MFC_KEY_A );

    if ( readed_bytes > 0 ) {
      uint8_t type;
      uint16_t size;
      uint8_t* content = tlv_decode ( buffer, &type, &size );
      free ( buffer );
      if ( type == 3 ) { // 3 == NDEF content type
        uint8_t rwpermissions = gpb | 0xf0;
        if ( rwpermissions == 0xf0 ) {
          if ( size == 0 ) {
            return NFCFORUM_INITIALISED;
          } else {
            ndefContent.append ( ( const char* ) content, ( int ) size );
            if ( !ndefContent.isEmpty() ) {
              NFCd::getNfcForumManager()->newAvailableNdefContent(ndefContent);
            }
            return NFCFORUM_READWRITE;
          }
        } else {
          if ( ( rwpermissions == 0xf3 ) && ( size != 0 ) ) {
            ndefContent.append ( ( const char* ) content, ( int ) size );
            if ( !ndefContent.isEmpty() ) {
              NFCd::getNfcForumManager()->newAvailableNdefContent(ndefContent);
            }
            return NFCFORUM_READONLY;
          } else return NFCFORUM_INVALID;
        }
      }
      free ( content );

    } else {
      qDebug ( "Unable to fetch content with this AID: fct=0xe1, app=0x03." );
      return NFCFORUM_INVALID;
    }
    mad_free ( mad );
  } else {
    return NFCFORUM_INVALID;
  }
}

NfcTarget::State
NfcTarget::mifare_ultralight_checkNFCForumTag()
{
  /*
  * NFC Forum type 2 tag
  */
  QByteArray CC;
  QByteArray tlv;
  uint8_t pageNum = 0;
  MifareUltralightPage pages[12];
  QByteArray ndefContent;

  if ( 0 != mifare_ultralight_read ( _tag, 0x03, & ( pages[0] ) ) ) return NFCFORUM_INVALID;
  CC.append ( ( const char * ) pages, 4 );

  // The CC area is set as described in section 6.1 with byte 3 equal to 00h (read/write access
  // granted)

  //Static memory structure
  if ( ( uint8_t ) CC.at ( 2 ) == 0x06 ) {
    if ( ( ( uint8_t ) CC.at ( 0 ) == 0xE1 ) && ( ( uint8_t ) CC.at ( 1 ) == 0x10 ) ) {
      if ( ( uint8_t ) CC.at ( 3 ) == 0x00 ) {
        // The data area contains an NDEF Message TLV
        for ( pageNum = 0x04; pageNum <= 0x0f; pageNum++ ) {
          mifare_ultralight_read ( _tag, pageNum, & ( pages[pageNum - 0x04] ) );
        }
        tlv.append ( ( const char * ) pages, 12 * sizeof ( MifareUltralightPage ) );
        // The length field of the NDEF Message TLV is equal to 00h.
        /*
        * Initialised state
        */
        if ( ( ( uint8_t ) tlv.at ( 0 ) == 0x03 ) ) {
          if ( ( uint8_t ) tlv.at ( 1 ) == 0x00 ) {
            return NFCFORUM_INITIALISED;
          } else {
            // The length field of the NDEF Message TLV is different from zero and equal to the actual
            //length of the NDEF message in the value field.
            ndefContent.append ( ( const char * ) pages, 12 * sizeof ( MifareUltralightPage ) );
            if ( !ndefContent.isEmpty() ) {
              NFCd::getNfcForumManager()->newAvailableNdefContent(ndefContent);
            }
            return NFCFORUM_READWRITE;
          }
        } else {
          return NFCFORUM_INVALID;
        }
      } else {
        // read only
        if ( ( uint8_t ) CC.at ( 3 ) == 0x0F ) {
          // The data area contains an NDEF Message TLV
          for ( pageNum = 0x04; pageNum <= 0x0f; pageNum++ ) {
            mifare_ultralight_read ( _tag, pageNum, & ( pages[pageNum - 0x04] ) );
          }
          tlv.append ( ( const char * ) pages, 12 * sizeof ( MifareUltralightPage ) );
          if ( ( ( uint8_t ) tlv.at ( 0 ) == 0x03 ) ) {
            if ( ( uint8_t ) tlv.at ( 1 ) != 0x00 ) {
              // The length field of the NDEF Message TLV is different from zero and equal to the actual
              //length of the NDEF message in the value field.
              ndefContent.append ( ( const char * ) pages, 12 * sizeof ( MifareUltralightPage ) );
              if ( !ndefContent.isEmpty() ) {
                NFCd::getNfcForumManager()->newAvailableNdefContent(ndefContent);
              }
              return NFCFORUM_READONLY;
            }
          }
        }
      }
    }
  }
  //TODO:Dynamic memory structure
  else {
    if ( ( ( uint8_t ) CC.at ( 2 ) == 0x10 ) || ( ( uint8_t ) CC.at ( 2 ) == 0xFF ) ) {
      qDebug ( "dynamic memory structure not yet implemented" );
    } else {
      return NFCFORUM_INVALID;
    }
  }
}

void
NfcTarget::mifare_desfire_formatted()
{
  if (0 == mifare_desfire_select_application(_tag, NULL)){
    uint8_t key_data_null[8]  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    MifareDESFireKey key;
    key = mifare_desfire_des_key_new_with_version (key_data_null);
    if (0 == mifare_desfire_authenticate(_tag, 0, key)){
      if (0 == mifare_desfire_format_picc(_tag)){
        qDebug("format_picc");
      }
    }
  }
}

// void
// NfcTarget::putMimeContent ( QString type, QByteArray data )
// {
//   // hack due to compatibility issue with the NFC phone used
//   type = type.contains ( "text/directory" ) ? "text/x-vCard" : type;
//   NDEFRecord rec = NDEFRecord::createMimeRecord ( type, data );
//   NDEFMessage msg ( rec );
//   this->writeNDEF ( msg );
// }

#define MIN(a,b) ((a < b) ? a: b)
int
NfcTarget::MifareClassicSearchSectorKey ( MifareClassicSectorNumber sector, MifareClassicKey *key, MifareClassicKeyType *key_type )
{
  // FIXME This only works with sector < 32.
  const MifareClassicBlockNumber block = sector * 4;
  mifare_classic_disconnect ( _tag );
  for ( size_t i = 0; i < ( sizeof ( default_keys ) / sizeof ( MifareClassicKey ) ); i++ ) {
    if ( ( 0 == mifare_classic_connect ( _tag ) ) && ( 0 == mifare_classic_authenticate ( _tag, block, default_keys[i], MFC_KEY_A ) ) ) {
      if ( ( 1 == mifare_classic_get_trailer_block_permission ( _tag, block + 3, MCAB_WRITE_KEYA, MFC_KEY_A ) ) &&
           ( 1 == mifare_classic_get_trailer_block_permission ( _tag, block + 3, MCAB_WRITE_ACCESS_BITS, MFC_KEY_A ) ) &&
           ( 1 == mifare_classic_get_trailer_block_permission ( _tag, block + 3, MCAB_WRITE_KEYB, MFC_KEY_A ) ) ) {
        memcpy ( key, &default_keys[i], sizeof ( MifareClassicKey ) );
        *key_type = MFC_KEY_A;
        return 1;
      }
    }
    mifare_classic_disconnect ( _tag );

    if ( ( 0 == mifare_classic_connect ( _tag ) ) && ( 0 == mifare_classic_authenticate ( _tag, block, default_keys[i], MFC_KEY_B ) ) ) {
      if ( ( ( block == 0 ) || ( 1 == mifare_classic_get_data_block_permission ( _tag, block + 0, MCAB_W, MFC_KEY_B ) ) ) &&
           ( 1 == mifare_classic_get_data_block_permission ( _tag, block + 1, MCAB_W, MFC_KEY_B ) ) &&
           ( 1 == mifare_classic_get_data_block_permission ( _tag, block + 2, MCAB_W, MFC_KEY_B ) ) &&
           ( 1 == mifare_classic_get_trailer_block_permission ( _tag, block + 3, MCAB_WRITE_KEYA, MFC_KEY_B ) ) &&
           ( 1 == mifare_classic_get_trailer_block_permission ( _tag, block + 3, MCAB_WRITE_ACCESS_BITS, MFC_KEY_B ) ) &&
           ( 1 == mifare_classic_get_trailer_block_permission ( _tag, block + 3, MCAB_WRITE_KEYB, MFC_KEY_B ) ) ) {
        memcpy ( key, &default_keys[i], sizeof ( MifareClassicKey ) );
        *key_type = MFC_KEY_B;
        return 1;
      }
    }
    mifare_classic_disconnect ( _tag );
  }

  warnx ( "No known authentication key for block %d", block );
  return 0;
}

int
NfcTarget::MifareClassicFixMadTrailerBlock ( MifareClassicSectorNumber sector, MifareClassicKey key, MifareClassicKeyType key_type )
{
  MifareClassicBlock block;
  if ( sector == 0 ) { // MAD Sector
    mifare_classic_trailer_block ( &block, mad_public_key_a, 0x0, 0x1, 0x1, 0x6, 0x00, default_key_b );
  } else {
    mifare_classic_trailer_block ( &block, mifare_classic_nfcforum_public_key_a, 0x0, 0x1, 0x1, 0x6, 0x00, default_key_b );
  }

  if ( mifare_classic_authenticate ( _tag, mifare_classic_sector_last_block ( sector ), key, key_type ) < 0 ) {
    qDebug() << "mifare_classic_authenticate failed. sector=" << sector << "key_type=" << key_type << "(MFC_KEY_B=" << MFC_KEY_B << ")" ;
    return -1;
  }
  if ( mifare_classic_write ( _tag, mifare_classic_sector_last_block ( sector ), block ) < 0 ) {
    qDebug() << "mifare_classic_write: error while writing trailer block";
    return -1;
  }
  return 0;
}

/**
 * Write NDEF message on this target
 * @param msg NDEF message to write on target
 */
void NfcTarget::writeNDEF ( const QByteArray msg )
{ qDebug("writeNDEF");
  // TODO: Add ISO14443B and Mifare Ultralight
  /* FIXME This function need full error handling rework. */
  MifareClassicKey key_00, key_10;
  MifareClassicKeyType key_00_type, key_10_type;
  Mad mad;
  _accessLock->lock();
  int error = 0;

  switch ( freefare_get_tag_type ( _tag ) ) {
    case CLASSIC_4K:
      if ( !MifareClassicSearchSectorKey ( 0x40, &key_10, &key_10_type ) ) {
        return;
      }
    case CLASSIC_1K:
      if ( !MifareClassicSearchSectorKey ( 0x00, &key_00, &key_00_type ) ) {
        return;
      }
      break;
    default:
      /* Keep compiler quiet */
      break;
  }

  if ( !error ) {
    /* Ensure the auth key is always a B one. If not, change it! */
    switch ( freefare_get_tag_type ( _tag ) ) {
      case CLASSIC_4K:
        if ( key_10_type != MFC_KEY_B ) {
          if ( 0 != MifareClassicFixMadTrailerBlock ( 0x40, key_10, key_10_type ) ) {
            error = 1;
            return;
          }
          memcpy ( &key_10, &default_key_b, sizeof ( MifareClassicKey ) );
          key_10_type = MFC_KEY_B;
        }
      case CLASSIC_1K:
        if ( key_00_type != MFC_KEY_B ) {
          if ( 0 != MifareClassicFixMadTrailerBlock ( 0x00, key_00, key_00_type ) ) {
            error = 1;
            return;
          }
          memcpy ( &key_00, &default_key_b, sizeof ( MifareClassicKey ) );
          key_00_type = MFC_KEY_B;
        }
        break;
      default:
        /* Keep compiler quiet */
        break;
    }
  }

  /* Now, we encode NDEF message in TLV format to know how many blocks is required */
  size_t encoded_size;
  const char* ndef_msg = msg.data();
  uint8_t *tlv_data = tlv_encode ( 3, ( uint8_t* ) ndef_msg, msg.size(), &encoded_size );

  if ( ! ( mad = mad_new ( ( freefare_get_tag_type ( _tag ) == CLASSIC_4K ) ? 2 : 1 ) ) ) {
    perror ( "mad_new" );
    error = 1;
    return;
  }

  MadAid aid;
  aid.function_cluster_code = 0xe1;
  aid.application_code = 0x03;

  MifareClassicSectorNumber *sectors = mifare_application_alloc ( mad, aid, encoded_size );
  if ( sectors == NULL ) {
    error = 1;
    return;
  }
  MifareClassicSectorNumber s;
  int i = 0;
  MifareClassicKey key;
  MifareClassicKeyType key_type;
  while ( s = sectors[i++] ) {
    if ( !MifareClassicSearchSectorKey ( s, &key, &key_type ) ) {
      error = 1;
      return;
    }

    if ( 0 != MifareClassicFixMadTrailerBlock ( s, key, key_type ) ) {
      error = 1;
      return;
    }
  }

  if ( mad_write ( _tag, mad, key_00, key_10 ) < 0 ) {
    perror ( "mad_write" );
    error = 1;
    return;
  }

  if ( mifare_application_write ( _tag, mad, aid, tlv_data, encoded_size, default_key_b, MFC_KEY_B ) < 0 ) {
    perror ( "mad_application_write" );
    error = 1;
    return;
  }
  
  free ( tlv_data );

  free ( mad );
  _accessLock->unlock();
  qDebug("NDEF wrote");
}

