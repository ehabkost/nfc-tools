#ifndef MIMECONTENTENTRY_H
#define MIMECONTENTENTRY_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QBuffer>
#include <QVarLengthArray>
#include <QMimeData>
#include "Entry.h"

/// MimeContentEntry
/**
  * MimeContentEntry is a subclass of Entry which handle
  * a mime entry
  */

class MimeContentEntry : public Entry {

	Q_OBJECT

	public:
		MimeContentEntry();
		MimeContentEntry(int,QString,QByteArray);
		MimeContentEntry(const MimeContentEntry& other);
		~MimeContentEntry();

		QByteArray* getData();

	protected:
		QByteArray* _data;

	private: 

};

#endif //MIMECONTENTENTRY_H
