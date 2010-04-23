#include "MimeContentEntry.h"

MimeContentEntry::MimeContentEntry() {
	_id = 0;
	_type = "";
	_data = new QByteArray();
}

MimeContentEntry::MimeContentEntry(int id,QString mimeType, QByteArray data)
  : Entry(id,mimeType)
 {
	_id = id;
	_data = new QByteArray();
	_data->append(data);
}

MimeContentEntry::MimeContentEntry( const MimeContentEntry& other) {
	_id = other._id;
	_type = other._type;
	_data = new QByteArray();
	_data->append( *(other._data));
}

MimeContentEntry::~MimeContentEntry() {
	delete _data;
}

QByteArray* MimeContentEntry::getData() {
	return _data;
}

