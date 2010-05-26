#include "NfcDeviceManager.h"
#include "nfcdevicemanageradaptor.h"

#include <QCoreApplication>
#include <QtDBus>
#include <QtDBus/QDBusConnection>
#include <QDebug>

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);

  NfcDeviceManager *nfcDeviceManager = new NfcDeviceManager();

  new NfcDeviceManagerAdaptor(nfcDeviceManager);

  QDBusConnection connection = QDBusConnection::systemBus();
  if( !connection.registerObject("/nfcd", nfcDeviceManager) ) {
    qFatal("Unable to register device manager on D-Bus.");
  }
//  connection.registerObject("/nfcd", nfcDeviceManager, QDBusConnection::ExportChildObjects);
  if ( !connection.registerService("org.nfc_tools.nfcd") ) {
    qDebug() <<  connection.lastError().message();
    qFatal("Unable to register service on D-Bus");
  }
  return app.exec();
}

