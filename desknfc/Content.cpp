#include "Content.h"

Content::Content(int id, QString type, QByteArray content) {
	_id= id;
	_type = type;
   _content = new QByteArray;
	_content->append(content);
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

