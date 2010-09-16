#include "NfcForumManager.h"
#include "nfcforumtagadaptor.h"
#include "NfcForumTag.h"
#include "NfcTarget.h"

#include <QDebug>
#include "NfcDevice.h"

NfcForumManager::NfcForumManager()
{
}

QStringList NfcForumManager::getNfcForumTagList()
{
  QStringList nfcForumTagUuids;
  for(int i=0; i<nfcForumTagUuids.size(); i++) {
    nfcForumTagUuids << _nfcForumTags.at(i)->getUuid();
  }
  return nfcForumTagUuids;
}

void NfcForumManager::newAvailableNdefContent(QByteArray NDEFmsg)
{
  emit ndefContentDetected(NDEFmsg);
}

void NfcForumManager::registerNfcForumTag(NfcForumTag* nfcForumTag)
{
  _nfcForumTags << nfcForumTag;
  connect(nfcForumTag, SIGNAL(destroyed(QObject*)), this, SLOT(unregisterNfcForumTag(QObject*)));
  new NfcForumTagAdaptor(nfcForumTag);
  QDBusConnection connection = QDBusConnection::systemBus();
  QString path = QString("/nfcd") + QString("/nfcForumTag_")
                + nfcForumTag->getUuid().toString().remove(QRegExp("[{}-]"));
  if( connection.registerObject(path, nfcForumTag) ) {
    qDebug() << "Forum Tag is D-Bus registred (" << path << ").";
         nfcForumTag->setPath(path);
    emit nfcForumTagAdded(nfcForumTag->getUuid());
  } else {
         qDebug() << connection.lastError().message();
    qFatal("Unable to register a new NFC Forum Tag on D-Bus.");
  }
}

void NfcForumManager::unregisterNfcForumTag(QObject* nfcForumTag)
{
  for(int i=0; i<_nfcForumTags.size(); i++) {
    if(_nfcForumTags.at(i) == (NfcForumTag*)nfcForumTag) {
      NfcForumTag* nfcForumTag = _nfcForumTags.takeAt(i);
      QDBusConnection connection = QDBusConnection::systemBus();
      QString path = QString("/nfcd") + QString("/nfcForumTag_")
        + nfcForumTag->getUuid().toString().remove(QRegExp("[{}-]"));
      connection.unregisterObject(path);

      qDebug() << "NFC Forum Tag is D-Bus unregistred (" << path << ").";
      emit nfcForumTagRemoved (nfcForumTag->getUuid());
      return ;
    }
  }
}

/*

void
NfcForumManager::initialiseMifareDESFireAsNFCForumTagType4(NfcTarget* target)
{
  // Before starting the INITIALISED Formatting Procedure a Mifare DESFire FormatPICC command
  // may be issued to the card to clear and release the whole memory from already present applications
  // and files.
//    mifare_desfire_formatted();

  // Send Mifare DESFire Select Application with AID equal to 000000h to select the PICC level
  if (0 == mifare_desfire_select_application(_tag, NULL)){
    qDebug ("PICC level selected");
    uint8_t key_data_null[8]  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    MifareDESFireKey key;
    key = mifare_desfire_des_key_new_with_version (key_data_null);


    // Authentication with PICC master key MAY be needed to issue ChangeKeySettings command
    if (0 == mifare_desfire_authenticate (_tag, 0, key)){
      qDebug ("Authenticated");
      uint8_t key_settings;
      uint8_t max_keys;
      mifare_desfire_get_key_settings(_tag, &key_settings,&max_keys);
      if ((key_settings & 0x08) == 0x08){

        // Send Mifare DESFire ChangeKeySetting to change the PICC master key settings into :
        // bit7-bit4 equal to 0000b
        // bit3 equal to Xb, the configuration of the PICC master key MAY be changeable or frozen
        // bit2 equal to 0b, CreateApplication and DeleteApplication commands are allowed with PICC master key authentication
        // bit1 equal to 0b, GetApplicationIDs, and GetKeySettings are allowed with PICC master key authentication
        // bit0 equal to Xb, PICC masterkey MAY be frozen or changeable
        if (0 == mifare_desfire_change_key_settings (_tag,0x00)){
          qDebug ("change the PICC master key settings");
        } else {
          qDebug ("NOT change the PICC master key settings");
          qDebug() << QString(desfire_error_lookup(mifare_desfire_get_last_error(_tag)));
        }
      }
        // Mifare DESFire Create Application with AID equal to EEEE10h, key settings equal to 09, NumOfKeys equal to 01h
//         MifareDESFireAID aid = mifare_desfire_aid_new(0xEE,0xEE,0x10);
        MifareDESFireAID aid = mifare_desfire_aid_new(0xEEEE10);

        if (0 == mifare_desfire_create_application (_tag, aid, 0x09, 1)){
          qDebug ("createApplication");

          // Mifare DESFire SelectApplication (Select previously creates application)
          if (0 == mifare_desfire_select_application(_tag, aid)){
            qDebug ("SelectApplication");
            free (aid);
            // Authentication with NDEF Tag Application master key (Authentication with key 0)
            if (0 == mifare_desfire_authenticate (_tag, 0, key)){
              qDebug ("Authentication with NDEF Tag Application master key");

              // Mifare DESFire ChangeKeySetting with key settings equal to 00000000b
              if (0 == mifare_desfire_change_key_settings (_tag,0x00)){
                qDebug ("ChangeKeySetting with key settings equal to 00000000");

                // Mifare DESFire CreateStdDataFile with FileNo equal to 03h (CC File DESFire FID), ComSet equal to 00h,
                // AccesRights equal to E000h, File Size bigger equal to 00000Fh
                if(0 == mifare_desfire_create_std_data_file(_tag,0x03,0x00,0xE000,0x00000F)){
                  qDebug ("CreateStdDataFile");

                  // Mifare DESFire WriteData to write the content of the CC File with CClEN equal to 000Fh,
                  // Mapping Version equal to 10h,MLe equal to 003Bh, MLc equal to 0034h, and NDEF File Control TLV
                  // equal to T =04h, L=06h, V=E1 04 (NDEF ISO FID=E104h) 0E E0 (NDEF File size =3808 Bytes) 00 (free read access)
                  // 00 free write access
                  byte_t CCFile_content[15] = { 0x00, 0x0F, //CCLEN
                                                0x10, // Mapping version
                                                0x00,0x3B, // MLe
                                                0x00,0x34, //MLc
                                                0x04, 0x06,0xE1,0x04, // TLV
                                                0x0E,0xE0, //NDEF File size
                                                0x00, //free read access
                                                0x00 // free write acces
                                               };
                  int res = mifare_desfire_write_data(_tag,0x03,0,sizeof(CCFile_content),CCFile_content);
                  if (res>0){
                    qDebug ("WriteData");

                    // Mifare DESFire CreateStdDataFile with FileNo equal to 04h (NDEF FileDESFire FID), CmmSet equal to 00h, AccessRigths
                    // equal to EEE0h, FileSize equal to 000EE0h (3808 Bytes)
                    if (0 == mifare_desfire_create_std_data_file(_tag,0x04,0x00,0xEEE0,0x000EE0)){
                      qDebug ("CreateStdDataFile");

                      //Mifare DESFire WriteData to write the content of the NDEF File with NLEN equal to 0000h and no NDEF Message
                      if (0 == mifare_desfire_write_data(_tag,0x04,0,0x00,NULL)){
                        qDebug ("WriteData");
                      }else qDebug ("NOT WriteData");
                    } else qDebug ("NOT CreateStdDataFile");
                  } else qDebug ("NOT WriteData");
                } else qDebug ("NOT CreateStdDataFile");
              } else qDebug ("NOT ChangeKeySetting with key settings equal to 00000000");
            } else qDebug ("NOT Authentication with NDEF Tag Application master key");
          } else qDebug ("NOT SelectApplication");
        } else qDebug ("NOT createApplication");
    } else qDebug ("NOT Authenticated");
    mifare_desfire_key_free (key);
  } else qDebug ("NOT Select the PICC level");
}

*/
