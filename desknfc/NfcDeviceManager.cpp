#include "NfcDeviceManager.h"

NfcDeviceManager::NfcDeviceManager()
{
  _iface = new OrgNfc_toolsNfcdNfcDeviceManagerInterface("org.nfc_tools.nfcd",
		"/nfcd", QDBusConnection::sessionBus(), this);
  if(!_iface->isValid()) qFatal("%s\n",NfcDeviceManager::tr("please launch nfcd").toStdString().c_str());
  else {
    QStringList devicesList = _iface->getDeviceList();
    for(int i=0; i<devicesList.size(); i++) {
      QString path = _iface->getDevicePathByName(devicesList.at(i)).value();
      NfcDevice* dev = new NfcDevice(path);
      _devicesName << dev->getName();
      _devices.append( dev );
    }
    QObject::connect(_iface, SIGNAL(devicePlugged(uchar,QString)),
      this, SLOT(addDevice(uchar,QString)) );
    QObject::connect(_iface, SIGNAL(deviceUnplugged(uchar,QString)),
      this, SLOT(removeDevice(uchar,QString)) );
  }
}
QStringList NfcDeviceManager::getDeviceList()
{
  /*QStringList devicesName;
  for(int i=0; i<_devices.size(); i++) {
    devicesName << _devices.at(i)->getName();
  }*/
  return _devicesName;
}

NfcDevice* NfcDeviceManager::getDeviceByName(QString deviceName) {
  NfcDevice* dev= NULL;
  for(int i=0; i<_devices.size(); i++) {
    if(deviceName == _devices.at(i)->getName()) 
		dev= _devices.at(i);
  }
  return dev;
}

void NfcDeviceManager::addDevice( uchar id, QString name) {
  (void)name;
  if(_devicesName.contains(name)) 
    removeDevice(id,name);
  NfcDevice* dev = new NfcDevice(_iface->getDevicePathById(id));
  _devicesName.append(dev->getName());
  _devices.append(dev);
  emit devicePlugged(id,name);
}

void NfcDeviceManager::removeDevice(uchar id,QString name) {
  (void)name; // no warning!
  for(int i=0; i<_devices.size(); i++) {
    if(id == _devices.at(i)->getId()) {
		_devices.removeAt(i);
		_devicesName.removeAll(name);
	 }
  }
  emit deviceUnplugged(id,name);
}

