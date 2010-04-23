#ifndef ENTRY_H
#define ENTRY_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QBuffer>
#include <QVarLengthArray>
#include <QMimeData>

class Entry : public QObject {

	Q_OBJECT

	public:
		Entry();
		Entry(int,QString);
		Entry(const Entry& other);

		~Entry();
		QString getType();
	   virtual int getId();
		virtual QByteArray* getData() = 0;

	protected:
		int _id;
		QString _type;

	private: 

};

#endif //ENTRY_H
