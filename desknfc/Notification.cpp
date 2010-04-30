#include "Notification.h"

Notification::Notification(QString eventId, Content* content, QString path)
  : KNotification(eventId) {
  _content = content;
  _path = path;
  if(content->getType().contains("(URI)")) {
    KUrl url(*(content->getData()));
		  setText(url.url());
    setPixmap( KIconLoader().loadIcon(
      KMimeType::iconNameForUrl(url), KIconLoader::Desktop));
  }
  else { 
    KFileItem fileItem(KFileItem::Unknown, KFileItem::Unknown, path);
    setText(tr("Type: ") + fileItem.mimeComment());
    setPixmap( KIconLoader().loadIcon(fileItem.iconName(), KIconLoader::Desktop ));
  }
}

void Notification::open() {
	QByteArray ba(*(_content->getData()));
	QString type = _content->getType();
	type.remove(QRegExp(".*: "));
	type.remove("\n");
	QDesktopServices::openUrl(
		_content->getType().contains("(URI)") ?
			QString(ba) : _path 
	);
}

/*Notification::~Notification() {

}*/
