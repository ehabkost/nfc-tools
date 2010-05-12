#ifndef NFCDEVICE_H
#define NFCDEVICE_H

#include <QObject>
#include <QStringList>
#include <QUuid>

#include "NfcDeviceInterface.h"
#include "NfcTarget.h"


/// NfcDevice
/** this class is basically an instance of a NFC Device
 * accessible through DBUS, provided by nfcd.
 * It is able to monitor the Device through it signals
 * and slots.
 */
class NfcDevice : public QObject
{
    Q_OBJECT

public:

  /// Constructor of NfcDevice
  /**
    * This constructor take the DBUS path of the NFC Device
    * and generate the object associated with this Device
    */
  NfcDevice(QString);
  ~NfcDevice();
  /// getter for _id
  uchar getId();

  /// getter for _name
  const QString getName();

  /// getter for the Uuid
  const QUuid getUuid();

  /// getter for _targets
  QList<NfcTarget*> getTargets();
  
public Q_SLOTS:

  /// get the list of the targets uids associated with this device
  QStringList getTargetList();

 /// get the NfcTarget object which have the given uid, if any
  NfcTarget* getTargetByUid(QString uid);

  /// add a target to this device
  void addTarget(QString,QString);

  /// remove a target to this device
  void removeTarget(QString,QString);

Q_SIGNALS:
  /// signal emitted when a new target enter the field of the device
  void targetFieldEntered(QString uid, QString name);

  /// signal emitted when a new target leave the field of the device
  void targetFieldLeft(QString uid, QString name);

protected:

  uchar _id;
  //QUuid _uuid;

  QString _name;

  /// DBUS interface to this Device provided by nfcd
  OrgNfc_toolsNfcdNfcDeviceInterface* _iface;
  
private:

  /// QList containing the targets associated with this device
  QList<NfcTarget*> _targets;
};

#endif // NFCDEVICE_H
