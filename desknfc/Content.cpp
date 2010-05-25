#include "Content.h"

Content::Content(int id, QString type, QByteArray content) {
	_id= id;
	_type = type;
   _content = new QByteArray;
	_content->append(content);
	_path = "";
}

Content::Content( const Content& other ) :
	QObject() {
	_id = other._id;
	_type = other._type;
	_content->append(*_content);
}

Content::~Content() {
  delete _content;
}


int Content::getId() {
	return _id;
}

QString Content::getType() {
	return _type;
}

QByteArray* Content::getData() {
	return _content;
}

QString Content::getPath() {
	return _path;
}

void Content::open() {
	QString type = _type;
	type.remove(QRegExp(".*: "));
	type = type.remove("\n");
	if(_type.contains("(URI)")) {
		QString url(*_content);
		QDesktopServices::openUrl(url);
	}
	else {
		if(_path == "") {
			_path = makeFile(*_content,_type);
		}
		//QDesktopServices::openUrl(_path);
		//KRun::runUrl(_path,type,NULL);
		KFileItem fileItem(KFileItem::Unknown, KFileItem::Unknown, _path);
		QString mimeType = fileItem.mimetype();
		KService::List lst = KMimeTypeTrader::self()->query(mimeType);
		//serviceName = lst.first()->desktopEntryName();
		if(!lst.isEmpty()) lst.first()->createInstance<QObject>();
	}
}

QString Content::getDesc() {
	if(_path == "") {
		_path = makeFile(*_content,_type);
	}
	QString desc = "";
	if(! _type.contains("(URI)") ) {
		KFileItem fileItem(KFileItem::Unknown, KFileItem::Unknown, _path);
		desc = fileItem.mimeComment();
	} else {
		KUrl url(*_content);
      desc = url.pathOrUrl();
	}
	return desc;
}

QString Content::getAssociatedServiceName() {
	QString serviceName = "";
	if(_path == "") {
		_path = makeFile(*_content,_type);
	}
	QString desc = "";
	if(! _type.contains("(URI)") ) {
		KFileItem fileItem(KFileItem::Unknown, KFileItem::Unknown, _path);
		QString mimeType = fileItem.mimetype();
		KService::List lst = KMimeTypeTrader::self()->query(mimeType);
		serviceName = !lst.isEmpty() ? lst.first()->desktopEntryName() : "unknown";
	}
	return serviceName;
}

QPixmap Content::getIcon() {
	if(_path == "") {
		_path = makeFile(*_content,_type);
	}
	QPixmap icon;
	if(! _type.contains("(URI)") ) {
		KFileItem fileItem(KFileItem::Unknown, KFileItem::Unknown, _path);
		icon = KIconLoader().loadIcon(fileItem.iconName(), KIconLoader::Desktop );
	} else {
		KUrl url(*_content);
      icon = KIconLoader().loadIcon(KMimeType::iconNameForUrl(url), KIconLoader::Desktop);
	}
	return icon;
}

QString Content::makeFile(QByteArray qb, QString mimeType) {
	(void)mimeType;
	QDir qd;
	qd.mkpath(QString("/tmp/desknfc-") + QString(getlogin()));
	QString path(QString("/tmp/desknfc-") + QString(getlogin())
	 + QString("/") + QUuid::createUuid().toString().remove(QRegExp("[{}-]")));
	QFile f(path);
	f.open(QIODevice::WriteOnly);
	f.write(qb);
	f.close();
	return path;
}

