#include "NfcDeviceManager.h"

#include <nfc/nfc.h>

#include "NfcDevice.h"
#include "nfcdeviceadaptor.h"

#define MAX_NFC_INITIATOR_COUNT 16

static nfc_device_desc_t known_devices_desc[MAX_NFC_INITIATOR_COUNT];

NfcDeviceManager::NfcDeviceManager()
{
  startTimer(1000);
  _accessLock = new QMutex(QMutex::Recursive);
}

QStringList NfcDeviceManager::getDeviceList()
{
  QStringList devicesName;
  for(int i=0; i<_devices.size(); i++) {
    devicesName << _devices.at(i)->getName();
  }
  return devicesName;
}

QString NfcDeviceManager::getDevicePathByName(QString deviceName) {
	QString devicePath = "";
  for(int i=0; i<_devices.size(); i++) {
    if(deviceName == _devices.at(i)->getName()) 
		devicePath = _devices.at(i)->getPath();
  }
  return devicePath;
}

QString NfcDeviceManager::getDevicePathById(uchar id) {
  QString devicePath = "";
  for(int i=0; i<_devices.size(); i++) {
    if(id  == _devices.at(i)->getId()) 
		devicePath = _devices.at(i)->getPath();
  }
  return devicePath;
}

void NfcDeviceManager::checkAvailableDevices()
{
  _accessLock->lock();
  // A new array is not required at each call (hence the 'static')
  static nfc_device_desc_t polled_devices_desc[MAX_NFC_INITIATOR_COUNT];
  size_t found = 0;

  nfc_list_devices (polled_devices_desc, MAX_NFC_INITIATOR_COUNT, &found);

  /* Look for disapeared devices */
  for (size_t i = 0; i< MAX_NFC_INITIATOR_COUNT; i++) {
    if (known_devices_desc[i].acDevice[0]) {
      bool still_here = false;
      for (size_t n = 0; n < found; n++) {
        if (0 == strcmp(known_devices_desc[i].acDevice, polled_devices_desc[n].acDevice))
          still_here = true;
      }
      if (!still_here) {
        unregisterDevice (i, known_devices_desc[i].acDevice);
        known_devices_desc[i].acDevice[0] = '\0';
      }
    }
  }

  /* Look for new devices */
  for (size_t i=0; i < found; i++) {
    bool already_known = false;
    int n;

    for (n=0; n<MAX_NFC_INITIATOR_COUNT; n++) {
      if (strcmp(polled_devices_desc[i].acDevice, known_devices_desc[n].acDevice) == 0)
        already_known = true;
    }

    if (!already_known) {
      for (n=0; n < MAX_NFC_INITIATOR_COUNT; n++) {
        if (known_devices_desc[n].acDevice[0] == '\0') {
          registerDevice(n, polled_devices_desc[i]);
          strncpy (known_devices_desc[n].acDevice , polled_devices_desc[i].acDevice,DEVICE_NAME_LENGTH);
          break;
        }
      }
      if (n == MAX_NFC_INITIATOR_COUNT) {
        qDebug ("MAX_NFC_INITIATOR_COUNT initiators reached.");
      }
    }
  }
  _accessLock->unlock();
}

void NfcDeviceManager::checkAvailableTargets()
{
  for(int i=0; i<_devices.size(); i++) {
    NfcDevice* nfcDevice = _devices.at(i);
    nfcDevice->checkAvailableTargets();
  }
}

void NfcDeviceManager::timerEvent(QTimerEvent *event)
{
  Q_UNUSED(event);
  checkAvailableDevices();
  checkAvailableTargets();
}

void NfcDeviceManager::registerDevice(uchar id, nfc_device_desc_t device)
{
  QString deviceName = QString(device.acDevice);
  qDebug() << "Register new device \"" << deviceName << "\" with ID: " << id << "...";
  NfcDevice* nfcDevice = new NfcDevice(id, device, _accessLock);
  _devices << nfcDevice;

  new NfcDeviceAdaptor(nfcDevice);

  QDBusConnection connection = QDBusConnection::sessionBus();
  QString path = QString("/nfcd") + QString("/device") 
	+ nfcDevice->getUuid().toString().remove(QRegExp("[{}-]"));
  qDebug() << "Trying to register \"" << deviceName << "\" at path: \"" << path << "\".";
  	if( connection.registerObject(path, nfcDevice) ) {
  	  qDebug() << "Device \"" << deviceName << "\" is D-Bus registred (" << path << ").";
    nfcDevice->setPath(path);
 	 emit devicePlugged(id, deviceName);
  } else {
    qDebug() << connection.lastError().message();
    qFatal("Unable to register a new device on D-Bus.");
  }
}

void NfcDeviceManager::unregisterDevice(uchar id, QString device)
{
  qDebug() << "Unregister device \"" << device << "\" with ID: " << id << "...";
  for(int i=0; i<_devices.size(); i++) {
    if(_devices.at(i)->getId() == id) {
      NfcDevice* nfcDevice = _devices.takeAt(i);
      QDBusConnection connection = QDBusConnection::sessionBus();
      QString path = QString("/nfcd") + QString("/device") 
        + nfcDevice->getUuid().toString().remove(QRegExp("[{}-]"));
      connection.unregisterObject(path);
		nfcDevice->setPath("");
      qDebug() << "Device \"" << device << "\" is D-Bus unregistred (" << path << ").";
      delete(nfcDevice);
      break;
    }
  }
  emit deviceUnplugged (id, device);
}
