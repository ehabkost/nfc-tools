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

class NfcDevice : public QObject
{
    Q_OBJECT

public:
  NfcDevice(const uchar, const nfc_device_desc_t, QMutex*);
  const QString getName();
  const QUuid getUuid();
  const QString getPath();
  void setPath(QString);

public Q_SLOTS:
  const uchar getId();
  void checkAvailableTargets();
  QStringList getTargetList();
  QString getTargetPathByUid(QString);

Q_SIGNALS:
  void targetFieldEntered(QString uid, QString name);
  void targetFieldLeft(QString uid, QString name);

protected:
  uchar _id;
  nfc_device_desc_t _device;
  QUuid _uuid;
  QString _dbusPath;
  QMutex* _accessLock;
  void timerEvent(QTimerEvent *event);

  void registerTarget(MifareTag tag);
  void unregisterTarget(NfcTarget* target);

private:
  QList<NfcTarget*> targets;
};

#endif // NFCDEVICE_H
