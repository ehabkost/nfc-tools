#ifndef NFCDEVICEMANAGER_H
#define NFCDEVICEMANAGER_H

#include <QObject>
#include <QList>
#include <QMutex>
#include <unistd.h>

#include "NfcDevice.h"

class NfcDeviceManager : public QObject
{
    Q_OBJECT

public:
  NfcDeviceManager();

public Q_SLOTS:
  QStringList getDeviceList();
  QString getDevicePathByName(QString);
  QString getDevicePathById(uchar);

Q_SIGNALS:
  void devicePlugged(uchar id, QString device);
  void deviceUnplugged(uchar id, QString device);

protected:
  void timerEvent(QTimerEvent *event);
  void checkAvailableDevices();
  void checkAvailableTargets();
  QMutex* _accessLock;

private:
  void registerDevice(uchar id, nfc_device_desc_t device);
  void unregisterDevice(uchar id, QString device);

  QList<NfcDevice*> _devices;
};

#endif // NFCDEVICEMANAGER_H
