#ifndef NFCFORUMTAG_H
#define NFCFORUMTAG_H

#include <QObject>
#include <QList>
#include <QMutex>
#include <unistd.h>
#include "NfcTarget.h"
#include <ndef/ndefmessage.h>

class NfcTarget;

class NfcForumTag : public QObject {
    Q_OBJECT

public:

    NfcForumTag(NfcTarget* );
    ~NfcForumTag();

    const QUuid getUuid();
    void setPath(const QString);
    const QString getPath();

public Q_SLOTS:

    void nfcTargetDestroyed(QObject*);
    void writeNDEF(const QByteArray);

protected:
    NfcTarget* _nfctarget;
    QString _path;
    QString _uuid;

private:

};

#endif // NFCFORUMTAG_H
