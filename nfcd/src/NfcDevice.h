#ifndef NFCDEVICE_H
#define NFCDEVICE_H

#include <QObject>
#include <QStringList>
#include <QUuid>
#include <QMutex>
#include <unistd.h>

#include <nfc/nfc.h>
#include <freefare.h>

class NfcTarget;

/// NfcDevice
/**
  * This class handle a NFC Device
  */

class NfcDevice : public QObject
{
    Q_OBJECT

public:
  /// construct a device with the given id, descriptor and mutex
  NfcDevice(const uchar, const nfc_device_desc_t, QMutex*);

  /// get device name
  const QString getName();

  /// getter for _uuid
  const QUuid getUuid();

  /// getter for _dbusPath
  const QString getPath();

  /// setter for _dbusPath
  void setPath(QString);

public Q_SLOTS:

  /// getter for _id
  const uchar getId();

  /// checking for new targets or missing targets
  void checkAvailableTargets();

  /// get the target list for this device
  QStringList getTargetList();

  /// get the DBUS path of the target with the given uid
  QString getTargetPathByUid(QString);

Q_SIGNALS:

  /// signal emitted when a target enter the field
  void targetFieldEntered(QString uid, QString name);

  /// signal emitted when a target leave the field
  void targetFieldLeft(QString uid, QString name);

protected:

  /// id of the device
  uchar _id;

  /// descriptor of the device
  nfc_device_desc_t _device;

  /// pointer to the device
  nfc_device_t* _device_connect;

  /// uuid of the device
  QUuid _uuid;

  /// DBUS path of this object
  QString _dbusPath;

  /// mutex protecting the access of the device
  QMutex* _accessLock;

  void timerEvent(QTimerEvent *event);

  /// register a new target
  void registerTarget(MifareTag tag);

  /// unregister a target
  void unregisterTarget(NfcTarget* target);

private:

  /// contain the targets of this device
  QList<NfcTarget*> targets;
};

#endif // NFCDEVICE_H
