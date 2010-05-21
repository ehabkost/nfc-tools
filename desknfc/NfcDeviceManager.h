#ifndef NFCDEVICEMANAGER_H
#define NFCDEVICEMANAGER_H

#include <QObject>
#include <QList>
#include <unistd.h>

#include "NfcDevice.h"
#include "NfcDeviceManagerInterface.h"

/// NfcDeviceManager

/** this class is basically an instance of a NFC Device Manager
 * accessible through DBUS, provided by nfcd.
 * It is able to monitor the Device Manager through it signals
 * and slots.
 */

class NfcDeviceManager : public QObject
{
    Q_OBJECT

public:
  NfcDeviceManager();

public Q_SLOTS:

  /// get the list of the devices uids founds by the device manager
  QStringList getDeviceList();

  /// get the NfcDevice object which have the given uid, if any
  NfcDevice* getDeviceByName(QString);

Q_SIGNALS:

  /// signal emitted when a new device is found
  void devicePlugged(uchar, QString);

  /// signal emitted when a device disappear
  void deviceUnplugged(uchar, QString);

protected:

private:

  /// DBUS interface to this DeviceManager provided by nfcd
  NfcDeviceManagerInterface* _iface;

  /// QList containing the NfcDevices objects
  QList<NfcDevice*> _devices;

  /// lists of the devices names
  QStringList _devicesName;

private slots:
  /// add a device
  void addDevice(uchar,QString);
  /// remove a device
  void removeDevice(uchar,QString);

};

#endif // NFCDEVICEMANAGER_H
