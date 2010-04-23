#ifndef URI_ENTRY_H
#define URI_ENTRY_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QBuffer>
#include <QVarLengthArray>
#include <QMimeData>

#include <ndef/ndefmessage.h>

#include "Entry.h"

class UriEntry : public Entry {

	Q_OBJECT

	public:
		UriEntry();
		UriEntry(int,QByteArray);
		UriEntry(const UriEntry& other);
		~UriEntry();

		QByteArray* getData();

	protected:
		QByteArray* _data;

	private: 

};

#endif // !URI_ENTRY_H
