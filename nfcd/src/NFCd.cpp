#include <QCoreApplication>
#include <QtDBus>
#include <QtDBus/QDBusConnection>
#include <QDebug>
#include "NFCd.h"
#include "NfcDeviceManager.h"
#include "NfcForumManager.h"
#include "nfcdevicemanageradaptor.h"
#include "nfcforummanageradaptor.h"

static NfcForumManager* _nfcForumManager;

NfcForumManager* NFCd::getNfcForumManager()
{
  return _nfcForumManager;
}

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);

  NfcDeviceManager *nfcDeviceManager = new NfcDeviceManager();

  new NfcDeviceManagerAdaptor(nfcDeviceManager);
//   _nfcForumManager = new NfcForumManager(...);
  _nfcForumManager = new NfcForumManager();
  new NfcForumManagerAdaptor(_nfcForumManager);

  QDBusConnection connection = QDBusConnection::systemBus();
  if( !connection.registerObject("/nfcd", nfcDeviceManager) ) {
    qFatal("Unable to register device manager on D-Bus.");
  }
  if( !connection.registerObject("/nfcd/NfcForumManager", _nfcForumManager) ) {
    qFatal("Unable to register nfc forum manager on D-Bus.");
  }
//  connection.registerObject("/nfcd", nfcDeviceManager, QDBusConnection::ExportChildObjects);
  if ( !connection.registerService("org.nfc_tools.nfcd") ) {
    qDebug() <<  connection.lastError().message();
    qFatal("Unable to register service on D-Bus");
  }
  return app.exec();
}

