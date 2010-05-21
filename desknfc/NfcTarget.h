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

/// NfcTarget
/** this class is basically an instance of a NFC Target
 * accessible through DBUS, provided by nfcd.
 * It is able to monitor the Target through it signals
 * and slots.
 */

class NfcTarget : public QObject
{
    Q_OBJECT

public:
  /// Constructor of NfcTarget
  /**
    * This constructor take the DBUS path of the NFC Target
    * and generate the object associated with this Target
    */
  NfcTarget(QString);
  ~NfcTarget();

  /// getter for the _uuid
  const QUuid getUuid();

  /// getter for the path
  const QString getPath();  // FIXME remove?

  void setPath(QString); // FIXME remove?

  /// get the contents list of this target
  const QLinkedList< QPair<QVariant,QString> >  getContentList();

public slots:

  /// get the content list as a string
  QStringList getContentListStrings();

  /// getter for _name
  QString getName();

  /// getter for _uid
  const QString getUid();

  /// get the content with the given id, if any
  Content* getContentById(int);

  /// get a QList containing the contents of this target
  QList<Content*> getTargetContent();

  /// write a file to this target
  void writeAFile(QByteArray,QString);

signals:
  /// signal emitted when a new content is available
  void newContentAvailable(int id, QString type);
  /// signal emitted when a content became unavailable
  void contentUnavailable(int id, QString type);

protected:

  QString _name;
  QString _uid;
  QString _uuid;

  /// QList containing the contents of this target
  QList<Content*> _targetContent;

  /// DBUS interface to the associated object provided by nfcd.
  NfcTargetInterface* _iface;

private:

};

#endif // NFCTARGET_H
