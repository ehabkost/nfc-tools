#ifndef NFCTARGET_H
#define NFCTARGET_H

// Qt
#include <QObject>
#include <QStringList>
#include <QList>
#include <QMap>
#include <QDir>
#include <QFile>
#include <QUuid>
#include <QMutex>

// C
#include <unistd.h>
#include <err.h>

// libnfc
#include <nfc/nfc.h>

// libfreefare
#include <freefare.h>

// internal
#include "iso14443b.h"

#include "NfcDevice.h"

/// NfcTarget
/**
  * This class handle a NFC Target
  */

class NfcTarget : public QObject {
    Q_OBJECT

public:
    enum Type {
      UNKNOWN,
      MIFARE_CLASSIC,
      MIFARE_ULTRALIGHT,
      MIFARE_DESFIRE,
      ISO14443_4A,
      ISO14443_4B,
    };

    /// construct a NfcTarget from the given tag and mutex
    NfcTarget(nfc_target_t, NfcDevice* );

    ~NfcTarget();

    /// getter for _uuid
    const QUuid getUuid();

    /// getter for _path
    const QString getPath();

    /// setter for _path
    void setPath( QString );

    /// getter for _name
    const QString getName();

    /// getter for _uid
    const QString getUid();

    /// get type
    const NfcTarget::Type getType();


public Q_SLOTS:


Q_SIGNALS:


protected:
    /// Target's type (see enum)
    NfcTarget::Type _type;

    /// name of this tag
    QString _name;
    /// uid of this tag
    QString _uid;
    /// uuid of this tag
    QUuid _uuid;
    /// D-Bus path of this tag
    QString _path;

    /// NfcDevice* parent
    NfcDevice* _device;

    /// libnfc's nfc_target_t struct
    nfc_target_t _nfc_target;

private:

    int MifareClassicSearchSectorKey( MifareClassicBlockNumber block, MifareClassicKey *key, MifareClassicKeyType *key_type );
    int MifareClassicFixMadTrailerBlock( MifareClassicSectorNumber sector, MifareClassicKey key, MifareClassicKeyType key_type );
};

#endif // NFCTARGET_H
