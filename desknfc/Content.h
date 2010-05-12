#ifndef CONTENT_H
#define CONTENT_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QDir>
#include <QUuid>
#include <QFile>
#include <QString>
#include <QUrl>
#include <QRegExp>
#include <QDesktopServices>

#include <KFileItem>
#include <KUrl>
#include <KIconLoader>
#include <KMimeType>

/// Content
/**
 * This class handle a Content as a ByteArray + id + type
 */

class Content : public QObject{

	Q_OBJECT

	public:
		Content(int, QString, QByteArray);
		Content(const Content& other);
		~Content();

		QString getType();
		QString getPath();
	   int getId();
		QString getDesc();
		QPixmap getIcon();
		QByteArray* getData();

	public slots:
		void open();


	protected:
		QByteArray* _content; 
		int _id;
		QString _type;
		QString _path;


	private: 
		QString makeFile(QByteArray, QString);

};

#endif // !CONTENT_H
