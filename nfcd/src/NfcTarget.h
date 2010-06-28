#ifndef NFCTARGET_H
#define NFCTARGET_H

#include <QObject>
#include <QStringList>
#include <QList>
#include <QMap>
#include <QDir>
#include <QFile>
#include <QUuid>
#include <QMutex>
#include <unistd.h>
#include <nfc/nfc.h>
#include <freefare.h>
#include <ndef/ndefmessage.h>
#include <err.h>

#include "MimeContentEntry.h"
#include "UriEntry.h"
#include "Content.h"

/// NfcTarget
/**
  * This class handle a NFC Target
  */

class NfcTarget : public QObject {
    Q_OBJECT

public:

    /// construct a NfcTarget from the given tag and mutex
    NfcTarget( MifareTag, QMutex* );
    ~NfcTarget();

    /// getter for _uuid
    const QUuid getUuid();

    /// getter for _path
    const QString getPath();

    /// setter for _path
    void setPath( QString );

    /// get the content list
    const QList< QPair<QVariant, QString> >  getContentList();

public slots:

    /// get the content list as a string list
    QStringList getContentListStrings();

    /// getter for _name
    QString getName();

    /// getter for _uid
    const QString getUid();

    /// get the ByteArray of the content with the given id
    QByteArray getContentById( int );

    /// put mime datas on the target
    void putMimeContent( QString, QByteArray );

signals:

    /// signal emitted when a new content is available
    void newContentAvailable( int id, QString type );

    /// signal emitted when trying to write a content bigger than card's capacity
    void contentTooBig();

protected:

    /// write a byte array to the target
    void putMessage( NDEFMessage msg );

    /// process a NDEF message
    void processNDEFMessage( NDEFMessage );

    /// process a Smart Poster
    Content* processSpMessage( int, NDEFMessage );

    /// check the target for contents
    void checkAvailableContent();

    /// search the key of a sector
    int search_sector_key( MifareTag, MifareClassicBlockNumber, MifareClassicKey *, MifareClassicKeyType * );

    /// put a MAD on the tag
    void putMad( NDEFMessage );

    /// stock this tag's MAD
    Mad _mad;
    /// sectors of this tag
    MifareSectorNumber* _sectors;
    /// MifareTag object of this tag, from libfreefare
    MifareTag _tag;
    /// name of this tag
    QString _name;
    /// uid of this tag
    QString _uid;
    /// uuid of this tag
    QUuid _uuid;
    /// _D-Bus path of this tag
    QString _path;
    /// content of this tag
    QList<Entry*> _targetContent;
    /// mutex locking access to this tag
    QMutex* _accessLock;

private:

    /// array with the default keys for Mifare Classic
    MifareClassicKey default_keys[8];

};

#endif // NFCTARGET_H
