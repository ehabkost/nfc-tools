#include "NfcTarget.h"

#include <QDebug>
#include <ndef/ndefrecordtype.h>

#include <stdlib.h>
#include <nfc/nfc.h>


NfcTarget::NfcTarget ( QString path )
{
  _iface = new NfcTargetInterface("org.nfc_tools.nfcd",
     path, QDBusConnection::sessionBus(), this);
  _uid = _iface->getUid();
  _uuid = _iface->getUuid();
  _name = _iface->getName();
  QStringList listContent = _iface->getContentListStrings();
  for(int i=0; i<listContent.size(); i++) { 
    QString id = listContent.at(i);
    id = id.remove( QRegExp(":.*") );
    QString type =  listContent.at(i);
    type = type.remove(QRegExp(".*: ")).remove("(MIME) ").remove("\n");
    QByteArray content = _iface->getContentById( id.toInt() );
    _targetContent.append( new Content(id.toInt(), type, content) );
  }
}

NfcTarget::~NfcTarget() {
  while(!_targetContent.empty()) delete _targetContent.takeLast();
  delete _iface;
}

QString NfcTarget::getName()
{
  return _name;
}

const QString NfcTarget::getUid()
{
  return _uid;
}

const QUuid NfcTarget::getUuid()
{
  return _uuid;
}

void NfcTarget::writeAFile(QByteArray data, QString type) {
  _iface->putMimeContent(type,data);
}

Content* NfcTarget::getContentById ( int i )
{
  Content* content= NULL;
  QList<Content*>::iterator it;
  for ( it=_targetContent.begin(); it!= _targetContent.end(); ++it ) {
    if ( ( *it )->getId() == i )
      content =  *it;
  }
  return content;
}

QList<Content*> NfcTarget::getTargetContent() {
	return _targetContent;
}

const QLinkedList< QPair<QVariant,QString> > NfcTarget::getContentList()
{
  QLinkedList< QPair<QVariant,QString> > ll;
  for(int i=0;i<_targetContent.size();i++) {
    ll.append ( QPair<QVariant,QString> ( _targetContent.at(i)->getId(),
      _targetContent.at(i)->getType() ) );
  }
  return ll;
}

QStringList NfcTarget::getContentListStrings()
{
  QStringList cl;
  QLinkedList< QPair<QVariant,QString> > ll = this->getContentList();
  QLinkedList< QPair<QVariant,QString> >::iterator it;
  for ( it = ll.begin(); it!= ll.end(); ++it ) {
    cl <<  QString::number ( ( *it ).first.toInt() ) + QString ( ": " )
    + ( *it ).second + QString ( "\n" );
  }
  return cl;
}
