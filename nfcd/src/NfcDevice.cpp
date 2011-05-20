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
  _nfc_device_desc = device;
  _nfc_device = nfc_connect(&_nfc_device_desc);
  _uuid = QUuid::createUuid();
  _dBusObjectPath = QDBusObjectPath("");
  _accessLock = accessLock;
}

/// get device name
const QString NfcDevice::getName()
{
  return QString(_nfc_device_desc.acDevice);
}

/// getter for _uuid
const QUuid NfcDevice::getUuid() {
  return _uuid;
}

/// return DBus path of this object
const QDBusObjectPath NfcDevice::getPath() {
  return _dBusObjectPath;
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
 * @brief setter for _dBusObjectPath
 * @param new D-Bus object path
 */
void NfcDevice::setPath(const QDBusObjectPath& path) {
  _dBusObjectPath = path;
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

#define MAX_TARGET_COUNT 8
/// checking for new targets or missing targets
void NfcDevice::checkAvailableTargets()
{
  _accessLock->lock();
  if(_nfc_device) {
    /* We are connected to NFC device */
    nfc_target_t nfc_targets[MAX_TARGET_COUNT];
    // List ISO14443A targets
    nfc_modulation_t nfc_modulation_iso14443a;
    nfc_modulation_iso14443a.nmt = NMT_ISO14443A;
    nfc_modulation_iso14443a.nbr = NBR_106;

    size_t targets_found_count;
    if (nfc_initiator_list_passive_targets (_nfc_device, nfc_modulation_iso14443a, nfc_targets, MAX_TARGET_COUNT, &targets_found_count)) {
      qDebug() << targets_found_count << "target(s) found.";
    } else {
      nfc_perror (_nfc_device, "nfc_initiator_list_passive_targets");
    }

    /* FIXME How to handle random UID ? */
    /* Look for disapeared devices */
    for(int i = 0; i < targets.size(); i++) {
      bool still_here = false;
      int j = 0;
      for (size_t n = 0; n < targets_found_count; n++) {
        QString uid(QByteArray ((const char*)nfc_targets[n].nti.nai.abtUid, nfc_targets[n].nti.nai.szUidLen).toHex());

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
    for (size_t n = 0; n < targets_found_count; n++) {
      QString uid(QByteArray ((const char*)nfc_targets[n].nti.nai.abtUid, nfc_targets[n].nti.nai.szUidLen).toHex());
  
      bool already_known = false;
      for(int j=0; j < targets.size(); j++) {
        if(targets.at(j)->getUid() == uid) {
          already_known = true;
          break;
        }
      }
      if(!already_known) {
        registerTarget(new NfcTarget(nfc_targets[n], this));
      }
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
    qDebug() << "Target \"" << target->getUid() << "\" is D-Bus registred (" << path << ").";
	 target->setPath(path);
    emit targetAdded(target->getUid(), target->getName());
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

      qDebug() << "Target \"" << uid << "\" is D-Bus unregistred (" << path << ").";
      delete(nfcTarget);

      break;
    }
  }
  emit targetRemoved (uid, name);
}

