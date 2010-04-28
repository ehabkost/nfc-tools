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

      /// get the title as a Byte Array
		QByteArray* getData();

      /// get the title grom the given language
		QString getTitle(QString);
      /// set the title for a language
      void setTitle(QString,QString);
		void addTitle(QByteArray);

	protected:

      /// stock the title as a pair of Languages and titles
		QList< QPair<QString,QString>* > _titles;


	private: 

};

#endif // !TITLE_ENTRY_H
