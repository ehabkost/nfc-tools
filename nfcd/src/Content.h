#ifndef CONTENT_H
#define CONTENT_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QBuffer>
#include <QVarLengthArray>
#include <QMimeData>

#include <ndef/ndefmessage.h>

#include "TitleEntry.h"
#include "Entry.h"

class Content : public Entry {

	Q_OBJECT

	public:
		Content(int);
		Content(int,QList<Entry*>);
		Content(const Content& other);
		~Content();

		void setType(QString);
		QString getType();
	   int getId();
		TitleEntry* getTitleEntry();
		void addEntry(Entry*);

		QList<Entry> getContents();
		QByteArray* getData();

	protected:
		QList<Entry*> _entries;
		TitleEntry* _te;


	private: 

};

#endif // !CONTENT_H
