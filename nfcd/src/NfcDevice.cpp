#include "NfcDevice.h"

#include "NfcTarget.h"
#include "nfctargetadaptor.h"
#include "NfcForumManager.h"
#include "NfcForumTag.h"
#include "NFCd.h"

#include <QDebug>
#include <QtDBus>
#include <QtDBus/QDBusConnection>

#include <stdlib.h>

/**
 * @brief construct a device with the given id, descriptor and mutex
 * @param deviceId id of the device
 * @param device the descriptor of the device
 * @param accessLock mutex locking this device's access
 */
NfcDevice::NfcDevice(const uchar deviceId, const nfc_device_desc_t device,
  QMutex* accessLock) : _id (deviceId)
{
  _device = device;
  _device_connect = nfc_connect(&_device);
  _uuid = QUuid::createUuid();
  _dbusPath = "";
  _accessLock = accessLock;
}

/// get device name
const QString NfcDevice::getName()
{
  return QString(_device.acDevice);
}

/// getter for _uuid
const QUuid NfcDevice::getUuid() {
  return _uuid;
}

/// getter for _dbusPath
const QString NfcDevice::getPath() {
	return _dbusPath;
}

/// getter for _id
const uchar NfcDevice::getId()
{
  return _id;
}

/// get the DBUS path of the target with the given uid
QString NfcDevice::getTargetPathByUid(QString tgUid)
{
  QString path = "";
  for(int i=0; i<targets.size(); i++) {
    if(tgUid == targets.at(i)->getUid()) 
		path = targets.at(i)->getPath();
  }
  return path;
}

/** 
 * @brief setter for _dbusPath
 * @param s new D-Bus path
 */
void NfcDevice::setPath(QString s) {
	_dbusPath = s;
} 

/// get the target list for this device
QStringList NfcDevice::getTargetList()
{
  QStringList targetUids;
  for(int i=0; i<targets.size(); i++) {
    targetUids << targets.at(i)->getUid();
  }
  return targetUids;
}


void NfcDevice::timerEvent(QTimerEvent *event)
{
  Q_UNUSED(event);

  qDebug ("NfcDevice::timerEvent");
}

/// checking for new targets or missing targets
void NfcDevice::checkAvailableTargets()
{
  _accessLock->lock();
  if(_device_connect) {
    /* We are connected to NFC device */
    MifareTag *mifareTags = NULL;
    mifareTags = freefare_get_tags (_device_connect);

    ISO14443bTag *iso14443bTags = NULL;
    iso14443bTags = iso14443b_get_tags(_device_connect);

    if ((mifareTags != NULL) && (iso14443bTags != NULL)) {
      int i = 0;
      MifareTag mifareTag;
      ISO14443bTag iso14443bTag;

      /* Look for disapeared devices */
      for(i = 0; i < targets.size(); i++) {
        bool still_here = false;
        int j = 0;
        if(targets.at(i)->getType() == MIFARE) {
          while(mifareTag = mifareTags[j]) {
            char* u = freefare_get_tag_uid(mifareTag);
            QString uid(u);
            free(u);

            if(targets.at(i)->getUid() == uid) {
              still_here = true;
              break;
            }
            j++;
          }
        } else if (targets.at(i)->getType() == ISO14443B) {
          // While ISO
          while(iso14443bTag = iso14443bTags[j]) {
            char* u = iso14443b_get_tag_uid(iso14443bTag);
            QString uid(u);
            free(u);

            if(targets.at(i)->getUid() == uid) {
              still_here = true;
              break;
            }
            j++;
          }
        }
        if(!still_here) {
          unregisterTarget(targets.at(i));
        }
      }

      /* Look for new devices */
      i = 0;
      // MIFARE
      while((mifareTag = mifareTags[i])) {
        char* u = freefare_get_tag_uid(mifareTag);
        QString uid(u);
        free(u);

        bool already_known = false;
        for(int j=0; j < targets.size(); j++) {
          if(targets.at(j)->getUid() == uid) {
            already_known = true;
            break;
          }
        }
        if(!already_known) {
          registerTarget(new NfcTarget(mifareTag, _device_connect, _accessLock));
        }
        i++;
      }
      // ISO14443B
      i = 0;
      while((iso14443bTag = iso14443bTags[i])) {
        char* u = iso14443b_get_tag_uid(iso14443bTag);
        QString uid(u);
        free(u);

        bool already_known = false;
        for(int j=0; j < targets.size(); j++) {
          if(targets.at(j)->getUid() == uid) {
            already_known = true;
            break;
          }
        }
        if(!already_known) {
          registerTarget(new NfcTarget(iso14443bTag, _device_connect, _accessLock));
        }
        i++;
      }
    } else {
      qDebug() << "Error while retrieving targets list (MIFARE or ISO14443B).";
    }
  }
  _accessLock->unlock();
}

void
NfcDevice::registerTarget(NfcTarget* target)
{
  targets << target;
  new NfcTargetAdaptor(target);

  QDBusConnection connection = QDBusConnection::systemBus();
  QString path = QString("/nfcd") + QString("/target_") 
		+ target->getUuid().toString().remove(QRegExp("[{}-]"));
  if( connection.registerObject(path, target) ) {
    qDebug() << "Target \"" << target->getName() << "\" is D-Bus registred (" << path << ").";
	 target->setPath(path);
    emit targetAdded(target->getUid(), target->getName());
    if (target->isNFCForumValidTag()) {
      NfcForumTag* _nfcForumTag = new NfcForumTag(target);
      NFCd::getNfcForumManager()->registerNfcForumTag(_nfcForumTag);
    }
  } else {
	 qDebug() << connection.lastError().message();
    qFatal("Unable to register a new Target on D-Bus.");
  }
}


void NfcDevice::unregisterTarget(NfcTarget* target)
{
  QString uid = target->getUid();
  QString name = target->getName();

  for(int i=0; i<targets.size(); i++) {
    if(targets.at(i) == target) {
      NfcTarget* nfcTarget = targets.takeAt(i);
      QDBusConnection connection = QDBusConnection::systemBus();
      QString path = QString("/nfcd") + QString("/target_") 
        + nfcTarget->getUuid().toString().remove(QRegExp("[{}-]"));
      connection.unregisterObject(path);

      qDebug() << "Target \"" << name << "\" is D-Bus unregistred (" << path << ").";
      delete(nfcTarget);

      break;
    }
  }
  emit targetRemoved (uid, name);
}

