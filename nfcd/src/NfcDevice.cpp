#include "NfcDevice.h"

#include "NfcTarget.h"
#include "nfctargetadaptor.h"

#include <QDebug>
#include <QtDBus>
#include <QtDBus/QDBusConnection>

#include <stdlib.h>

NfcDevice::NfcDevice(const uchar deviceId, const nfc_device_desc_t device,
  QMutex* accessLock) : _id (deviceId)
{
  _device = device;
  _uuid = QUuid::createUuid();
  _dbusPath = "";
  _accessLock = accessLock;	
}

const QString NfcDevice::getName()
{
  return QString(_device.acDevice);
}

const QUuid NfcDevice::getUuid() {
  return _uuid;
}

const QString NfcDevice::getPath() {
	return _dbusPath;
}

const uchar NfcDevice::getId()
{
  return _id;
}

QString NfcDevice::getTargetPathByUid(QString tgUid)
{
  QString path = "";
  for(int i=0; i<targets.size(); i++) {
    if(tgUid == targets.at(i)->getUid()) 
		path = targets.at(i)->getPath();
  }
  return path;
}

void NfcDevice::setPath(QString s) {
	_dbusPath = s;
} 

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

void NfcDevice::checkAvailableTargets()
{
  _accessLock->lock();
  nfc_device_t * nfc_device = NULL;
  nfc_device = nfc_connect(&_device);

  if(nfc_device) {
    /* We are connected to NFC device */
    MifareTag *tags = NULL;
    tags = freefare_get_tags (nfc_device);
    if (tags == NULL) {
      nfc_disconnect (nfc_device);
      nfc_device = NULL;
    } else {
      int i = 0;
      MifareTag tag;

      /* Look for disapeared devices */
      for(i = 0; i < targets.size(); i++) {
        bool still_here = false;
        int j = 0;
        while((tag = tags[j])) {
          char* u = freefare_get_tag_uid(tag);
          QString uid(u);
          free(u);

          if(targets.at(i)->getUid() == uid) {
            still_here = true;
            break;
          }
          j++;
        }
        if(!still_here) {
          unregisterTarget(targets.at(i));
        }
      }

      /* Look for new devices */
      i = 0;
      while((tag = tags[i])) {
        char* u = freefare_get_tag_uid(tag);
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
          registerTarget(tag);
        }
        i++;
      }

      freefare_free_tags (tags);
      tags = NULL;
      nfc_disconnect (nfc_device);
      nfc_device = NULL;
    }
  }
  _accessLock->unlock();
}

void
NfcDevice::registerTarget(MifareTag tag)
{
  NfcTarget* nfcTarget = new NfcTarget(tag, _accessLock);
  targets << nfcTarget;
  new NfcTargetAdaptor(nfcTarget);

  QDBusConnection connection = QDBusConnection::sessionBus();
  QString path = QString("/nfcd") + QString("/target_") 
		+ nfcTarget->getUuid().toString().remove(QRegExp("[{}-]"));
  if( connection.registerObject(path, nfcTarget) ) {
    qDebug() << "Device \"" << nfcTarget->getName() << "\" is D-Bus registred (" << path << ").";
	 nfcTarget->setPath(path);
    emit targetFieldEntered(nfcTarget->getUid(), nfcTarget->getName());
  } else {
	 qDebug() << connection.lastError().message();
    qFatal("Unable to register a new device on D-Bus.");
  }

}

void NfcDevice::unregisterTarget(NfcTarget* target)
{
  QString uid = target->getUid();
  QString name = target->getName();

  for(int i=0; i<targets.size(); i++) {
    if(targets.at(i) == target) {
      NfcTarget* nfcTarget = targets.takeAt(i);
      QDBusConnection connection = QDBusConnection::sessionBus();
      QString path = QString("/nfcd") + QString("/target_") 
        + nfcTarget->getUuid().toString().remove(QRegExp("[{}-]"));
      connection.unregisterObject(path);

      qDebug() << "Device \"" << name << "\" is D-Bus unregistred (" << path << ").";
      delete(nfcTarget);
      break;
    }
  }
  emit targetFieldLeft (uid, name);
}

