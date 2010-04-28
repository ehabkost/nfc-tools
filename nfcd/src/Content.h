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

/// Content
/**
  *This class represent contents found on targets
  */
class Content : public Entry {

	Q_OBJECT

	public:

      /// construct the content with the given id
		Content(int);

      /// construct the content with the given id and list of entries
		Content(int,QList<Entry*>);

		Content(const Content& other);
		~Content();

      /// setter for _type
		void setType(QString);

      /// getter for _type
		QString getType();

      /// getter for id
	   int getId();

      /// get the title entry
		TitleEntry* getTitleEntry();

      /// add an entry to _entries
		void addEntry(Entry*);

      /// get the entries
		QList<Entry> getContents();

      /// get the content as a ByteArray
		QByteArray* getData();

	protected:

      /// stock the entries
		QList<Entry*> _entries;

      /// title entry of this content
		TitleEntry* _te;


	private: 

};

#endif // !CONTENT_H
