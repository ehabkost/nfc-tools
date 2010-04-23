#ifndef CONTENT_H
#define CONTENT_H

#include <QObject>
#include <QString>
#include <QByteArray>

class Content : public QObject{

	Q_OBJECT

	public:
		Content(int, QString, QByteArray);
		Content(const Content& other);
		~Content();

		QString getType();
	   int getId();
		QByteArray* getData();

	protected:
		QByteArray* _content; 
		int _id;
		QString _type;


	private: 

};

#endif // !CONTENT_H
