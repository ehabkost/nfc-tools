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

typedef enum {
  MIFARE,
  ISO14443B
} tag_type;

/// NfcTarget
/**
  * This class handle a NFC Target
  */

class NfcTarget : public QObject {
    Q_OBJECT

  enum State {
    UNKNOWN,
    NFCFORUM_INITIALISED,
    NFCFORUM_READWRITE,
    NFCFORUM_READONLY,
    NFCFORUM_INVALID
  };

public:

    /// construct a NfcTarget from the given tag and mutex
    NfcTarget(ISO14443bTag, nfc_device_t*, QMutex* );
    NfcTarget(MifareTag, nfc_device_t*, QMutex* );

    ~NfcTarget();

    /// getter for _uuid
    const QUuid getUuid();

    /// getter for _path
    const QString getPath();

    /// setter for _path
    void setPath( QString );

    /// check the tag is compliant with NFC Forum
    bool isNFCForumValidTag();

    /// get the content list as a string list
    QStringList getContentListStrings();

    /// getter for _name
    const QString getName();

    /// getter for _uid
    const QString getUid();

    /// get type
    const tag_type getType();

    /// get the ByteArray of the content with the given id
    QByteArray getContentById( int );

    void writeNDEF( const QByteArray );


public Q_SLOTS:

    /// put mime datas on the target
//     void putMimeContent( QString, QByteArray );

Q_SIGNALS:

    /// signal emitted when a new content is available
//     void newContentAvailable( int id, QString type );

protected:
    /// process a NDEF message
//      void processNDEFMessage( NDEFMessage );

    /// process a Smart Poster
//      Content* processSpMessage( int, NDEFMessage );

    /// check the target for qualifying them into NFC Forum if so.
    void checkNfcForumTag();

    /// check the target for contents
    void checkAvailableContent();

    /// check the state (INITIALISED, READ/WRITE, READONLY, INVALID)
    NfcTarget::State iso7816_4_checkNFCForumTag();
    NfcTarget::State mifare_classic_checkNFCForumTag();
    NfcTarget::State mifare_ultralight_checkNFCForumTag();
    NfcTarget::State mifare_desfire_checkNFCForumTag();

    /// search the key of a sector
    int searchSectorKey( MifareTag, MifareClassicBlockNumber, MifareClassicKey *, MifareClassicKeyType * );

    void mifare_desfire_formatted();

    MifareTag _tag;
    ISO14443bTag _tag14443b;
    tag_type _type;
    nfc_device_t* _device;

    /// name of this tag
    QString _name;
    /// uid of this tag
    QString _uid;
    /// uuid of this tag
    QUuid _uuid;
    /// _D-Bus path of this tag
    QString _path;
    /// mutex locking access to this tag
    QMutex* _accessLock;

    NfcTarget::State _state;

private:

    int MifareClassicSearchSectorKey( MifareClassicBlockNumber block, MifareClassicKey *key, MifareClassicKeyType *key_type );
    int MifareClassicFixMadTrailerBlock( MifareClassicSectorNumber sector, MifareClassicKey key, MifareClassicKeyType key_type );
};

#endif // NFCTARGET_H
