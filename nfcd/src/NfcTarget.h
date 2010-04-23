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

class NfcTarget : public QObject
{
    Q_OBJECT

public:
  NfcTarget(MifareTag, QMutex*);
  ~NfcTarget();
  const QUuid getUuid();
  const QString getPath();
  void setPath(QString);
  void checkAvaibleContent(); // TODO make protected

  const QList< QPair<QVariant,QString> >  getContentList();

public slots:
  QStringList getContentListStrings();
  QString getName();
  const QString getUid();
  QByteArray getContentById(int);
  void putMimeContent(QString, QByteArray);

signals:
  void newContentAvailable(int id,QString type);
  

protected:
  void putContent(QByteArray);
  void processNDEFMessage(NDEFMessage);
  Content* processSpMessage(int, NDEFMessage);

  Mad _mad;
  MifareSectorNumber* _sectors;
  MifareTag _tag;
  QString _name;
  QString _uid;
  QUuid _uuid;
  QString _path;
  QList<Entry*> _targetContent;
  QMutex* _accessLock;

private:
};

#endif // NFCTARGET_H
