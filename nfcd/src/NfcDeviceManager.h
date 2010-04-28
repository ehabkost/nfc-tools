#ifndef NFCDEVICEMANAGER_H
#define NFCDEVICEMANAGER_H

#include <QObject>
#include <QList>
#include <QMutex>
#include <unistd.h>

#include "NfcDevice.h"

/// NfcDeviceManager
/**
  * This class manage the NFC Devices connected to the computer
  */

class NfcDeviceManager : public QObject
{
    Q_OBJECT

public:
  NfcDeviceManager();

public Q_SLOTS:

  /// get the list of the devices
  QStringList getDeviceList();

  /// get the device with the given name
  QString getDevicePathByName(QString);

  /// get the device with the given id
  QString getDevicePathById(uchar);

Q_SIGNALS:

  /// signal emitted when a device is plugged
  void devicePlugged(uchar id, QString device);

  /// signal emitted when a device is unplugged
  void deviceUnplugged(uchar id, QString device);

protected:

  void timerEvent(QTimerEvent *event);

  /// check for availables devices
  void checkAvailableDevices();

  /// check for availables targets
  void checkAvailableTargets();  // FIXME responsability of NfcDevice?

  /// mutex protecting the access to the device
  QMutex* _accessLock;

private:

  /// register a new device
  void registerDevice(uchar id, nfc_device_desc_t device);

  /// unregister a device
  void unregisterDevice(uchar id, QString device);


  /// QList containing the devices
  QList<NfcDevice*> _devices;
};

#endif // NFCDEVICEMANAGER_H
