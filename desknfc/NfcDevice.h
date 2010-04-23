#ifndef NFCDEVICE_H
#define NFCDEVICE_H

#include <QObject>
#include <QStringList>
#include <QUuid>

#include "NfcDeviceInterface.h"
#include "NfcTarget.h"

class NfcDevice : public QObject
{
    Q_OBJECT

public:
  NfcDevice(QString);
  ~NfcDevice();
  uchar getId();
  const QString getName();
  const QUuid getUuid();
  
public Q_SLOTS:
  QStringList getTargetList();
  NfcTarget* getTargetByUid(QString uid);
  void addTarget(QString,QString);
  void removeTarget(QString,QString);

Q_SIGNALS:
  void targetFieldEntered(QString uid, QString name);
  void targetFieldLeft(QString uid, QString name);

protected:
  uchar _id;
  //QUuid _uuid;
  QString _name;
  OrgNfc_toolsNfcdNfcDeviceInterface* _iface;
  
private:
  QList<NfcTarget*> _targets;
};

#endif // NFCDEVICE_H
