#include "UriEntry.h"

UriEntry::UriEntry() {
	_id = 0;
	_type = "";
	_data = new QByteArray();
}

UriEntry::UriEntry(int id, QByteArray data)
  : Entry(id,"Uri") {
	_data = new QByteArray();
   _data->append(NDEFRecord::uriProtocol(data));
	_data->append(data.right(data.size()-1));
}

UriEntry::UriEntry( const UriEntry& other) {
	_id = other._id;
	_type = other._type;
	_data = new QByteArray();
	_data->append( *(other._data));
}

UriEntry::~UriEntry() {
	delete _data;
}

QByteArray* UriEntry::getData() {
	return _data;
}


