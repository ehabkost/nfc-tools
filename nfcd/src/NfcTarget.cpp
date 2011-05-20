#include "NfcTarget.h"

#include <QDebug>
#include <ndef/ndefrecordtype.h>

#include <stdlib.h>
#include <nfc/nfc.h>
#include "NFCd.h"
#include "NfcForumTag.h"
#include "nfcforum_type4_tag.h"

/*
 * This implementation was written based on information provided by the
 * following documents:
 *
 * NXP Type MF1K/4K Tag Operation
 * Storing NFC Forum data in Mifare Standard 1k/4k
 * Rev. 1.1 - 21 August 2007
 */

NfcTarget::NfcTarget ( nfc_target_t nfc_target, NfcDevice* device ):
  _nfc_target(nfc_target),
  _device(device),
  _uid(QByteArray ((const char*)nfc_target.nti.nai.abtUid, nfc_target.nti.nai.szUidLen).toHex())
{
  _uuid = QUuid::createUuid();
}

NfcTarget::~NfcTarget()
{
}

const QString NfcTarget::getName()
{
  return _name;
}

const NfcTarget::Type NfcTarget::getType()
{
  return _type;
}

const QString NfcTarget::getUid()
{
  return _uid;
}

const QUuid NfcTarget::getUuid()
{
  return _uuid;
}

const QString NfcTarget::getPath()
{
  return _path;
}

void NfcTarget::setPath ( QString path )
{
  _path = path;
}
