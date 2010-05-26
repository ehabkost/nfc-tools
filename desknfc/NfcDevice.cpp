#include "NfcDevice.h"

#include <QDebug>
#include <QtDBus>
#include <QtDBus/QDBusConnection>

#include <stdlib.h>

NfcDevice::NfcDevice(QString path) {
  _iface = new NfcDeviceInterface("org.nfc_tools.nfcd",
			path, QDBusConnection::systemBus(), this);
  if( _iface->isValid() ) {
    _name = _iface->getName();
    _id = _iface->getId();
    QObject::connect(_iface,SIGNAL(targetFieldEntered(QString,QString)),
      this, SLOT(addTarget(QString,QString)));
    QObject::connect(_iface,SIGNAL(targetFieldLeft(QString,QString)),
      this, SLOT(removeTarget(QString,QString)));
    QStringList targetsList = _iface->getTargetList();
    for(int i = 0; i < targetsList.size(); i++) {
      QString path = _iface->getTargetPathByUid(targetsList.at(i));
      _targets.append( new NfcTarget(path) );
    }
  }
  else {
    qDebug() << "DBus interface not valid with path: (" + path + ")";
  }
}

NfcDevice::~NfcDevice() {
   QList<NfcTarget*>::iterator it;
  for(it = _targets.begin(); it != _targets.end(); ++it) {
    delete *it;
  }
  delete _iface;

}

const QString NfcDevice::getName()
{
  return _name;
}

QList<NfcTarget*> NfcDevice::getTargets() {
  return QList<NfcTarget*>(_targets);
}

void NfcDevice::addTarget(QString uid, QString name) {
  QString path = _iface->getTargetPathByUid(uid); 
  _targets.append( new NfcTarget(path) );
  emit targetFieldEntered(uid,name);
}

void NfcDevice::removeTarget(QString uid, QString name) {
  int size = _targets.size();
  for(int i=0; i< size; i++) {
    if( uid == _targets.at(i)->getUid() ) { 
      //delete _targets.at(i);
		_targets.removeAt(i);
		i--;
      size--;
    }
  }
  emit targetFieldLeft(uid,name);
}

/*const QUuid NfcDevice::getUuid() {
  return _uuid;
}*/

uchar NfcDevice::getId()
{
  return _id;
}

NfcTarget* NfcDevice::getTargetByUid(QString tgUid)
{
  NfcTarget* target = NULL;
  for(int i=0; i<_targets.size(); i++) {
    if(tgUid == _targets.at(i)->getUid()) 
		target = _targets.at(i);
  }
  return target;
}

QStringList NfcDevice::getTargetList()
{
  QStringList targetUids;
  for(int i=0; i<_targets.size(); i++) {
    targetUids << _targets.at(i)->getUid();
  }
  return targetUids;
}

