#include "NfcForumTag.h"

#include <QDebug>

#include <stdlib.h>
#include "NfcTarget.h"

NfcForumTag::NfcForumTag(NfcTarget* nfcTarget)
{
  _nfctarget = nfcTarget;
  _uuid = nfcTarget->getUuid();
  connect( nfcTarget, SIGNAL(destroyed(QObject*)), this, SLOT(nfcTargetDestroyed(QObject*)));
}

NfcForumTag::~NfcForumTag()
{
}

void 
NfcForumTag::writeNDEF(const QByteArray ndef)
{
  _nfctarget->writeNDEF(ndef);
}

void
NfcForumTag::nfcTargetDestroyed(QObject*nfcTarget)
{
  this->deleteLater();
}

const QUuid NfcForumTag::getUuid()
{
  return _uuid;
}

const QString NfcForumTag::getPath()
{
  return _path;
}

void NfcForumTag::setPath(const QString path)
{
  _path = path;
}

// TODO This function must be placed in DeskNFC
// void
// NfcForumTag::putMimeContent ( QString type, QByteArray data )
// {
//  qDebug("putMimeContent");
//  // hack due to compatibility issue with the NFC phone used
//  type = type.contains ( "text/directory" ) ? "text/x-vCard" : type;
//  NDEFRecord rec = NDEFRecord::createMimeRecord ( type, data );
//  NDEFMessage msg ( rec );
//  _nfctarget->writeNDEF(msg);
//}

