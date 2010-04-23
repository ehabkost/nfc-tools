#ifndef NFCTARGET_H
#define NFCTARGET_H

#include <QObject>
#include <QStringList>
#include <QLinkedList>
#include <QMainWindow>
#include <QMap>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QUuid>

#include "Content.h"
#include "NfcTargetInterface.h"

class NfcTarget : public QObject
{
    Q_OBJECT

public:
  NfcTarget(QString);
  ~NfcTarget();
  const QUuid getUuid();
  const QString getPath();
  void setPath(QString);
  const QLinkedList< QPair<QVariant,QString> >  getContentList();

public slots:
  QStringList getContentListStrings();
  QString getName();
  const QString getUid();
  Content* getContentById(int);
  QList<Content*> getTargetContent();
  void writeAFile(QByteArray,QString);

signals:
  void newContentAvailable(int id, QString type);
  void contentUnavailable(int id, QString type);

protected:
  QString _name;
  QString _uid;
  QString _uuid;
  QList<Content*> _targetContent;
  OrgNfc_toolsNfcdNfcTargetInterface* _iface;

private:

};

#endif // NFCTARGET_H
