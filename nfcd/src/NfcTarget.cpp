#include "NfcTarget.h"

#include <QDebug>
#include <ndef/ndefrecordtype.h>

#include <stdlib.h>
#include <nfc/nfc.h>


NfcTarget::NfcTarget ( MifareTag tag, QMutex* accessLock )
{
  _accessLock = accessLock;
  _tag = tag;
  char* u = freefare_get_tag_uid ( _tag );
  _uid = QString ( u );
  _uuid = QUuid::createUuid();
  free ( u );
  _name = freefare_get_tag_friendly_name ( _tag );
  _targetContent = QList<Entry*>();
  checkAvailableContent();
  default_keys = {
    { 0xff,0xff,0xff,0xff,0xff,0xff },
    { 0xd3,0xf7,0xd3,0xf7,0xd3,0xf7 },
    { 0xa0,0xa1,0xa2,0xa3,0xa4,0xa5 },
    { 0xb0,0xb1,0xb2,0xb3,0xb4,0xb5 },
    { 0x4d,0x3a,0x99,0xc3,0x51,0xdd },
    { 0x1a,0x98,0x2c,0x7e,0x45,0x9a },
    { 0xaa,0xbb,0xcc,0xdd,0xee,0xff },
    { 0x00,0x00,0x00,0x00,0x00,0x00 }
  };


}

NfcTarget::~NfcTarget() {
  QList<Entry*>::iterator it;
  for(it = _targetContent.begin(); it != _targetContent.end(); ++it) {
    delete *it;
  }
  freefare_free_tag(_tag);
}

QString NfcTarget::getName()
{
  return _name;
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

QByteArray NfcTarget::getContentById ( int i )
{
  QByteArray content;
  QList<Entry*>::iterator it;
  for ( it=_targetContent.begin(); it!= _targetContent.end(); ++it ) {
    if ( ( *it )->getId() == i )
      content = * ( ( *it )->getData() );
  }
  return content;
}

const QList< QPair<QVariant,QString> > NfcTarget::getContentList()
{
  QList< QPair<QVariant,QString> > cl;
  QList<Entry*>::iterator it;
  for ( it = _targetContent.begin(); it != _targetContent.end(); ++it ) {
    cl.append ( QPair<QVariant,QString> ( ( *it )->getId(), ( *it )->getType() ) );
  }
  return cl;
}

QStringList NfcTarget::getContentListStrings()
{
  QStringList sl;
  QList< QPair<QVariant,QString> > cl = this->getContentList();
  QList< QPair<QVariant,QString> >::iterator it;
  for ( it = cl.begin(); it!= cl.end(); ++it ) {
    sl <<  QString::number ( ( *it ).first.toInt() ) + QString ( ": " )
    + ( *it ).second + QString ( "\n" );
  }
  return sl;
}

#define block(s,b) ((s * 4) + b)
void
NfcTarget::checkAvailableContent()
{
  _accessLock->lock();
  QByteArray data;
  int resConnect = -1;

  enum mifare_tag_type type = freefare_get_tag_type(_tag);
  switch(type) { 
    case CLASSIC_1K:
    case CLASSIC_4K:

      resConnect = mifare_classic_connect ( _tag );
      if ( 0 == resConnect ) {

        Mad mad = mad_read ( _tag );
        MadAid aid;
        aid.function_cluster_code = 0xE1;
        aid.application_code = 0x03;

        MifareClassicKey key = { 0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7 };

        if ( mad != NULL ) {
          MifareSectorNumber* sectors = mifare_application_find ( mad, aid );
          if ( sectors != NULL ) {
            int i = 0;
            MifareSectorNumber sector;
            while ( ( sector = sectors[i] ) ) {
              qDebug() << "Dump sector " << QString::number ( sector ) << "...";
              if ( 0 == mifare_classic_authenticate ( _tag, block ( sector, 0 ), key, MFC_KEY_A ) ) {
                for ( uint8_t b=0; b<3; b++ ) {
                  MifareClassicBlock r;
                  if ( 0 == mifare_classic_read ( _tag, block ( sector, b ), &r ) ) {
                    data.append ( ( char* ) r, sizeof ( MifareClassicBlock ) );
                  } else {
                    qDebug ( "Unable to read block." );
                  }
                }
              } else {
                qDebug ( "Unable to authenticate on sector." );
              }
              i++;
            }
          } else {
            qDebug ( "No sector for aid 0x03, 0xE1" );
          }
        mad_free ( mad );
        } else {
          qDebug ( "Unable to read MAD." );
        }
        if( mifare_classic_disconnect ( _tag ) == 0 )
        qDebug() << "disconnected";
      } else {
        qDebug() << "Unable to connect to mifare classic tag.";
      }
      _accessLock->unlock();
    break;
    case ULTRALIGHT:
      resConnect =  mifare_ultralight_connect (_tag);
      if(0 == resConnect) {
        uint8_t pageNum = 0;
        MifareUltralightPage pages[12];
        for(pageNum = 0x04; pageNum <= 0x0f; pageNum++) {
          mifare_ultralight_read (_tag, pageNum, &(pages[pageNum - 0x04]) );
        }
        data.append( (const char *)pages, 12 * sizeof(MifareUltralightPage) );

        if( mifare_ultralight_disconnect ( _tag ) == 0 )
          qDebug() << "disconnected";
        _accessLock->unlock();
      }
    break;
  }
  if ( !data.isEmpty() ) {
    QString hex;
    //dump
        /*QDir qd;
        qd.mkpath(QString("/tmp/nfcd-dumps-") + QString( getlogin() ) );
        QString path(QString("/tmp/nfcd-dumps-") + QString( getlogin() )
           + QString("/") + QUuid::createUuid().toString().remove(QRegExp("[{}-]")));
        QFile f(path);
        f.open(QIODevice::WriteOnly);
        qDebug() << f.write(data);*/
      //pmud
    for ( int n = 0; n<data.size(); n++ ) {
      if ( ( uint8_t ) data.at ( n ) < 0x10 )
        hex = hex + "0";

      hex = hex + QString::number ( ( uint8_t ) data.at ( n ), 16 );
      if ( ( n%16 ) == 15 ) hex = hex + "\n"; else hex = hex + " ";
    }

    qDebug() << hex;
    // Test if content is an NDEF message
    // TLV according to "Type 1 Tag Operation Specification" from NFCForum

    uint16_t tlv_len;
    if ( ( uint8_t ) data.at ( 0 ) == 0x03 ) {
      if ( ( uint8_t ) data.at ( 1 ) != 0xff ) {
        // TLV use 1 byte for lenght
        tlv_len = data.at ( 1 );
        data.remove ( 0, 2 ); // Remove the first 2 bytes (corresponding to "Tag" and "Lenght" from TLV format)
      } else {
        // TLV use 3 bytes for lenght
        tlv_len = ( uint8_t ) data.at ( 3 );
        tlv_len |= ( ( ( uint16_t ) data.at ( 2 ) ) << 8 );
        data.remove ( 0, 4 ); // Remove the first 4 bytes (corresponding to "Tag" (1 byte) and "Lenght" (3 bytes) from TLV format)
      }

      data.truncate ( tlv_len );

      // Now we can parse it...
      NDEFMessage msg = NDEFMessage::fromByteArray ( data );
      processNDEFMessage ( msg );
    }
  
  }
}

void NfcTarget::putMessage(NDEFMessage msg) {
  _accessLock->lock();
  bool needAlloc = false;
  QByteArray data = msg.toByteArray();
  enum mifare_tag_type type = freefare_get_tag_type(_tag);
  if( (type == CLASSIC_1K && data.size() >= 720) 
    ||  (type == CLASSIC_4K && data.size() >= 3840) ) {  
    // TODO: is 3840 the right capacity for a 4K?
    emit contentTooBig();
    qDebug() << "content too big, don't write";
    return;
  }
  /*uint16_t tlv_len = data.size();
  uint8_t tlv_lenR = (uint8_t) (tlv_len & 0xff);
  uint8_t tlv_lenL = (uint8_t) (tlv_len >> 8);
  data.prepend( tlv_lenR );
  data.prepend( tlv_lenL);
  data.prepend((char)0xff);
  data.prepend( (char)0x03 );*/
  data.append( (char)0xfe ); 
  if ( 0 == mifare_classic_connect ( _tag ) ) {
    Mad mad = mad_read ( _tag );
    if(mad == NULL) {
        needAlloc = true;
        mifare_classic_connect ( _tag );
        putMad(msg);
        qDebug() << "new MAD written on tag";
    }
    MadAid aid;
    aid.function_cluster_code = 0xE1;
    aid.application_code = 0x03;
    size_t size = data.size() / 16;
    if(size % 16) size++;
    if(needAlloc) {
      mifare_application_alloc(mad, aid, size);
    }
    MifareClassicKey key = { 0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7 };

    if ( mad != NULL ) {
      MifareSectorNumber* sectors = mifare_application_find ( mad, aid );
      if ( sectors != NULL ) {
        int i = 0;
        MifareSectorNumber sector;
        while ( ( sector = sectors[i] ) ) {
          qDebug() << "Writing sector " << QString::number ( sector ) << "...";
          if ( 0 == mifare_classic_authenticate ( _tag, block ( sector, 0 ), key, MFC_KEY_A ) ) {
            for ( uint8_t b=0; b<3; b++ ) {
              QByteArray tmpArray;
              MifareClassicBlock r={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
					for( int j=0; (i*48)+(b*16)+j < data.size() && j<16; j++ ) {
						r[j] = data[ (i*48)+(b*16)+j ];
					}
              if ( 0 == mifare_classic_write ( _tag, block ( sector, b ), r ) ) {
                //data.append ( ( char* ) r, sizeof ( MifareClassicBlock ) );
              } else {
                qDebug ( "Unable to write block." );
              }
            }
          } else {
            qDebug ( "Unable to authenticate on sector." );
          }
          i++;
        }
      } else {
        qDebug ( "No sector for aid 0x03, 0xE1" );
      }
      mad_free ( mad );
    } else {
      qDebug ( "Unable to read MAD." );
    }
  } else {
    qDebug() << "Unable to connect to mifare classic tag.";
  }
  warn(NULL);
  if( mifare_classic_disconnect ( _tag ) == 0 )
    qDebug() << "disconnected";
  _accessLock->unlock();
}

void
NfcTarget::putMimeContent(QString type, QByteArray data)
{
  // hack due to compatibility issue with the NFC phone used
  type = type.contains("text/directory") ? "text/x-vCard" : type;
  NDEFRecord rec = NDEFRecord::createMimeRecord(type,data);
  NDEFMessage msg(rec);
  putMessage(msg);
}

int
NfcTarget::search_sector_key (MifareTag tag, MifareClassicBlockNumber block, MifareClassicKey *key, MifareClassicKeyType *key_type)
{
    mifare_classic_disconnect (tag);
    for (int i = 0; i < (sizeof (default_keys) / sizeof (MifareClassicKey)); i++) {
	if ((0 == mifare_classic_connect (tag)) && (0 == mifare_classic_authenticate (tag, block, default_keys[i], MFC_KEY_A))) {
	    if ((1 == mifare_classic_get_trailer_block_permission (tag, block + 3, MCAB_WRITE_KEYA, MFC_KEY_A)) &&
		(1 == mifare_classic_get_trailer_block_permission (tag, block + 3, MCAB_WRITE_ACCESS_BITS, MFC_KEY_A)) &&
		(1 == mifare_classic_get_trailer_block_permission (tag, block + 3, MCAB_WRITE_KEYB, MFC_KEY_A))) {
		memcpy (key, &default_keys[i], sizeof (MifareClassicKey));
		*key_type = MFC_KEY_A;
		return 1;
	    }
	}
   mifare_classic_disconnect (tag);
	if ((0 == mifare_classic_connect (tag)) && (0 == mifare_classic_authenticate (tag, block, default_keys[i], MFC_KEY_B))) {
	    if (((block == 0) || (1 == mifare_classic_get_data_block_permission (tag, block + 0, MCAB_W, MFC_KEY_B))) &&
		(1 == mifare_classic_get_data_block_permission (tag, block + 1, MCAB_W, MFC_KEY_B)) &&
		(1 == mifare_classic_get_data_block_permission (tag, block + 2, MCAB_W, MFC_KEY_B)) &&
		(1 == mifare_classic_get_trailer_block_permission (tag, block + 3, MCAB_WRITE_KEYA, MFC_KEY_B)) &&
		(1 == mifare_classic_get_trailer_block_permission (tag, block + 3, MCAB_WRITE_ACCESS_BITS, MFC_KEY_B)) &&
		(1 == mifare_classic_get_trailer_block_permission (tag, block + 3, MCAB_WRITE_KEYB, MFC_KEY_B))) {
		memcpy (key, &default_keys[i], sizeof (MifareClassicKey));
		*key_type = MFC_KEY_B;
		return 1;
	    }
	}
  mifare_classic_disconnect (tag);
    }

    warnx ("No known authentication key for block %d", block);
    return 0;
}

#define MIN(a,b) ((a < b) ? a: b)
void NfcTarget::putMad(NDEFMessage msg) 
{
    int error = 0;
    nfc_device_t *device = NULL;
    //MifareTag *tags = NULL;
    MifareClassicKey key_00, key_01, key_10;
    MifareClassicKeyType key_00_type, key_01_type, key_10_type;
    MifareClassicBlock block;
    Mad mad;

	 size_t encoded_size;
	 off_t pos = 0;
	 uint8_t *tlv_data = tlv_encode (3, (uint8_t*)msg.toByteArray().data(), msg.toByteArray().size(), &encoded_size);
	 MifareClassicBlockNumber bn = 0x04;


    switch (freefare_get_tag_type(_tag) ) {
		case CLASSIC_4K:
		    if (!search_sector_key (_tag, 0x40, &key_10, &key_10_type)) {
			error = 1;
			goto error;
		    }
		case CLASSIC_1K:
		    if (!search_sector_key (_tag, 0x00, &key_00, &key_00_type)) {
			error = 1;
			goto error;
		    }
		    break;
		default:
		    /* Keep compiler quiet */
		    break;
	    }


	    if (!error) {
		/* Ensure the auth key is always a B one. If not, change it! */
		switch ( freefare_get_tag_type(_tag) ) {
		    case CLASSIC_4K:
			if (key_10_type != MFC_KEY_B) {
			    mifare_classic_trailer_block (&block, default_keys[2], 0x0, 0x1, 0x1, 0x6, 0x00, default_keys[1]);
			    if (mifare_classic_authenticate (_tag, 0x10, key_10, key_10_type) < 0) {
				perror ("mifare_classic_authenticate 1");
				error = 1;
				goto error;
			    }
			    if (mifare_classic_write (_tag, 0x43, block) < 0) {
				perror ("mifare_classic_write");
				error = 1;
				goto error;
			    }
			    memcpy (&key_10, &(default_keys[1]), sizeof (MifareClassicKey));
			    key_10_type = MFC_KEY_B;
			}
		    case CLASSIC_1K:
			if (key_00_type != MFC_KEY_B) {
			    mifare_classic_trailer_block (&block, default_keys[2], 0x0, 0x1, 0x1, 0x6, 0x00, default_keys[1]);
			    if (mifare_classic_authenticate (_tag, 0x00, key_00, key_00_type) < 0) {
				perror ("mifare_classic_authenticate 2");
				error = 1;
				goto error;
			    }
			    if (mifare_classic_write (_tag, 0x03, block) < 0) {
				perror ("mifare_classic_write");
				error = 1;
				goto error;
			    }
			    memcpy (&key_00, &(default_keys[1]), sizeof (MifareClassicKey));
			    key_00_type = MFC_KEY_B;
			}
			break;
		    default:
			/* Keep compiler quiet */
			break;
		}
	    }

	    if (!(mad = mad_new ((freefare_get_tag_type (_tag) == CLASSIC_4K) ? 2 : 1))) {
		perror ("mad_new");
		error = 1;
		goto error;
	    }

	    MadAid aid;
		 aid.function_cluster_code = 0xe1;
		 aid.application_code = 0x03;
	   

	    mad_set_aid (mad, 1, aid);

	    if (mad_write (_tag, mad, key_00, key_10) < 0) {
		perror ("mad_write");
		error = 1;
		goto error;
	    }




	    if (mifare_classic_authenticate (_tag, bn, key_01, key_01_type) < 0) {
		perror ("mifare_classic_authenticate 3");
		error = 1;
		goto error;
	    }

	    MifareClassicBlock data;

	    while (pos < encoded_size) {
		memset (&data, '\0', sizeof (MifareClassicBlock));
		memcpy (&data, tlv_data + pos, MIN (encoded_size - pos, sizeof (MifareClassicBlock)));
		pos += sizeof (MifareClassicBlock);
		if (bn == 0x07)
		    abort();
		if (mifare_classic_write (_tag, bn++, data) < 0) {
		    perror ("mifare_classic_write");
		    error = 1;
		    goto error;
		}
	    }

       mifare_classic_trailer_block (&block, default_keyb, 0x0, 0x0, 0x0, 0x6, 0x40, default_keyb);
	    if (mifare_classic_write (tags[i], 0x07, block) < 0) {
		perror ("mifare_classic_write");
		error = 1;
		goto error;
	    }

	    free (tlv_data);

	    free (mad);
     error:
	    //free (tag_uid);
       return;
       
}


void
NfcTarget::processNDEFMessage ( NDEFMessage msg )
{
  // ...and then we can use it.
  if ( msg.isValid() ) {
    qDebug() << "Num of records:" << msg.recordCount();
    for ( int i=0; i<msg.recordCount(); i++ ) {
      NDEFRecord record = msg.record ( i );
      if ( record.isValid() ) {
        qDebug() << "msg.record(" << i << ").type().id() = " << record.type().id();
        if ( NDEFRecordType::NDEF_MIME == record.type().id() ) {
          qDebug() << "MIME type:" << record.type().name();
        } else if ( NDEFRecordType::NDEF_URI == record.type().id() ) {
          qDebug() << "URI identifier code: " << record.type().name();
        } else if ( NDEFRecordType::NDEF_NfcForumRTD == record.type().id() ) {
          qDebug() << "Well known type: " << record.type().name();
        } else if ( NDEFRecordType::NDEF_ExternalRTD == record.type().id() ) {
          qDebug() << "External type: " << record.type().name();
        } else if ( NDEFRecordType::NDEF_Empty == record.type().id() ) {
          qDebug() << "Empty" ;
        } else if ( NDEFRecordType::NDEF_Unknown == record.type().id() ) {
          qDebug() << "Unknown";
        } else if ( NDEFRecordType::NDEF_Unchanged == record.type().id() ) {
          qDebug() << "Invalid";
        }
        QByteArray record_payload = msg.record ( i ).payload();
        if ( !record_payload.isEmpty() ) {
          if ( NDEFRecordType::NDEF_MIME == record.type().id() ) {
            MimeContentEntry *nc = new
              MimeContentEntry ( i, QString("(MIME) ") + record.type().name(),record_payload );
            _targetContent.append ( nc );
          } else if ( NDEFRecordType::NDEF_NfcForumRTD == record.type().id()
                      && record.type().name() == QString ( "Sp" ) ) {
            qDebug() << QString ( "SmartPoster !" ) ;
            Content *c = processSpMessage (i, NDEFMessage::fromByteArray ( record.payload() ) );
            _targetContent.append(c);
          } else if( NDEFRecordType::NDEF_NfcForumRTD == record.type().id() &&
             record.type().name() == "U" ) {
             qDebug("URI");
             NDEFRecord uri(record.payload());
             UriEntry  *nuc = new UriEntry(i,uri.payload());
             _targetContent.append(nuc);
          } else if( NDEFRecordType::NDEF_NfcForumRTD == record.type().id() &&
             record.type().name() == "T" ) {
             NDEFRecord titleRecord( record.payload() );
             QString locale = QString::fromLocal8Bit(NDEFRecord::textLocale(record.payload()));
             QString title = QString::fromLocal8Bit(record.payload());
             title = title.right(title.length()-locale.length() - 1);
             MimeContentEntry *nc = new
               MimeContentEntry ( i,QString ( "text/plain" ),title.toUtf8() );
             _targetContent.append(nc);
          } else {
            MimeContentEntry *nc = new
              MimeContentEntry ( i,QString ( "application/octet-stream" ),record_payload );
            _targetContent.append ( nc );
          }
          emit newContentAvailable ( i, QString(_name) + " : " +record.type().name() );
        }
        qDebug ( "Payload:" );
        QString hex;
        for ( int n = 0; n<record_payload.size(); n++ )
          hex = hex + QString ( "0x" ) + QString::number ( ( unsigned char ) record_payload.at ( n ), 16 ) + " ";

        qDebug() << hex;
        QString content = QString ( record_payload.data () );
        qDebug() << content;
      } else {
        qDebug() << "NDEFRecord is not valid.";
      }
    }
    qDebug() << "content list: " << this->getContentListStrings();
  } else {
    qDebug() << "This is not a valid NDEF message.";
  }
}

Content* NfcTarget::processSpMessage(int id, NDEFMessage msg) {
    Content *res = new Content(id);
    QList<Entry*> contents;
    if ( msg.isValid() ) {
    qDebug() << "Num of records:" << msg.recordCount();
    for ( int i=0; i<msg.recordCount(); i++ ) {
      NDEFRecord record = msg.record ( i );
      if ( record.isValid() ) {
        qDebug() << "msg.record(" << i << ").type().id() = " << record.type().id();
        if ( NDEFRecordType::NDEF_MIME == record.type().id() ) {
          qDebug() << "MIME type:" << record.type().name();
        } else if ( NDEFRecordType::NDEF_URI == record.type().id() ) {
          qDebug() << "URI identifier code: " << record.type().name();
        } else if ( NDEFRecordType::NDEF_NfcForumRTD == record.type().id() ) {
          qDebug() << "Well known type: " << record.type().name();
        } else if ( NDEFRecordType::NDEF_ExternalRTD == record.type().id() ) {
          qDebug() << "External type: " << record.type().name();
        } else if ( NDEFRecordType::NDEF_Empty == record.type().id() ) {
          qDebug() << "Empty" ;
        } else if ( NDEFRecordType::NDEF_Unknown == record.type().id() ) {
          qDebug() << "Unknown";
        } else if ( NDEFRecordType::NDEF_Unchanged == record.type().id() ) {
          qDebug() << "Invalid";
        }
        QByteArray record_payload = msg.record ( i ).payload();
        if ( !record_payload.isEmpty() ) {
          if ( NDEFRecordType::NDEF_MIME == record.type().id() ) {
            MimeContentEntry *nc = new
              MimeContentEntry ( i,QString("(MIME) ") + record.type().name(),record_payload );
            res->addEntry( nc );
          } else if ( NDEFRecordType::NDEF_NfcForumRTD == record.type().id()
                      && record.type().name() == QString ( "Sp" ) ) {
            qDebug() << QString ( "SmartPoster !" );
            Content* c = processSpMessage (i, NDEFMessage::fromByteArray ( record.payload() ) );
            res->addEntry(c);
          } else if( NDEFRecordType::NDEF_NfcForumRTD == record.type().id() &&
             record.type().name() == "U" ) {
              qDebug("URI");
              NDEFRecord uri(record.payload());
              UriEntry* nuc =  new UriEntry(i,uri.payload());
              res->addEntry(nuc);
			} else if( NDEFRecordType::NDEF_NfcForumRTD == record.type().id() &&
             record.type().name() == "T" ) {
              NDEFRecord titleRecord( record.payload() );
              TitleEntry* te = res->getTitleEntry();
              QString locale = QString(NDEFRecord::textLocale(record.payload()));
              QString title = QString(record.payload());
              title = QString("(URI) ") + title.right(title.length()-locale.length() - 1);
              res->setType(title);
              te->setTitle(locale, title);
         } else {
            MimeContentEntry *nc = new
              MimeContentEntry ( i,QString ( "application/octet-stream" ),record_payload );
            res->addEntry( nc );
          }
        }
        qDebug ( "Payload:" );
        QString hex;
        for ( int n = 0; n<record_payload.size(); n++ )
          hex = hex + QString ( "0x" ) + QString::number ( ( unsigned char ) record_payload.at ( n ), 16 ) + " ";

        qDebug() << hex;
        QString content = QString ( record_payload.data () );
        qDebug() << content;
      } else {
        qDebug() << "NDEFRecord is not valid.";
      }
    }
    qDebug() << "content list: " << this->getContentListStrings();
  } else {
    qDebug() << "This is not a valid NDEF message.";
  }
  return res;
}
