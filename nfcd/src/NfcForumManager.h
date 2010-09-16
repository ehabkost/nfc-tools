#ifndef NFCFORUMMANAGER_H
#define NFCFORUMMANAGER_H

#include <QObject>
#include <QList>
#include <QMutex>
#include <unistd.h>
#include <ndef/ndefmessage.h>
#include <nfc/nfc-types.h>

#include "NfcForumTag.h"
#include "NfcTarget.h"

class NfcForumManager : public QObject
{
    Q_OBJECT

public:

  NfcForumManager();

  void newAvailableNdefContent(QByteArray NDEFmsg);
  void registerNfcForumTag(NfcForumTag* nfcForumTag);

public Q_SLOTS:

  QStringList getNfcForumTagList();
  void unregisterNfcForumTag(QObject*);

Q_SIGNALS:

  void ndefContentDetected(QByteArray NDEFmsg);
  void nfcForumTagAdded(QString uid);
  void nfcForumTagRemoved(QString uid);

protected:



private:
  QList<NfcForumTag*> _nfcForumTags;
};

#endif // NFCFORUMMANAGER_H
