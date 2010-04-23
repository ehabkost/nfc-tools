#ifndef TITLE_ENTRY_H
#define TITLE_ENTRY_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QBuffer>
#include <QList>
#include <QVarLengthArray>
#include <QMimeData>
#include <qdebug.h>

#include <ndef/ndefmessage.h>

#include "Entry.h"

class TitleEntry : public Entry {

	Q_OBJECT

	public:
		TitleEntry();
		TitleEntry(const TitleEntry& other);
		~TitleEntry();

		QByteArray* getData();
		QString getTitle(QString);
      void setTitle(QString,QString);
		void addTitle(QByteArray);

	protected:
		QList< QPair<QString,QString>* > _titles;


	private: 

};

#endif // !TITLE_ENTRY_H
