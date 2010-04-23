#ifndef NFCDEVICEMANAGER_H
#define NFCDEVICEMANAGER_H

#include <QObject>
#include <QList>
#include <unistd.h>

#include "NfcDevice.h"
#include "NfcDeviceManagerInterface.h"

class NfcDeviceManager : public QObject
{
    Q_OBJECT

public:
  NfcDeviceManager();

public Q_SLOTS:
  QStringList getDeviceList();
  NfcDevice* getDeviceByName(QString);

Q_SIGNALS:
  void devicePlugged(uchar, QString);
  void deviceUnplugged(uchar, QString);

protected:

private:
  OrgNfc_toolsNfcdNfcDeviceManagerInterface* _iface;
  QList<NfcDevice*> _devices;
  QStringList _devicesName;

private slots:
  void addDevice(uchar,QString);
  void removeDevice(uchar,QString);

};

#endif // NFCDEVICEMANAGER_H
