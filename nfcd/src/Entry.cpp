#include "Entry.h"

Entry::Entry() {
	_id = 0;
	_type = "";
}

Entry::Entry(int id,QString type)
 {
	_id = id;
	_type = type;
}

Entry::Entry( const Entry& other) {
	_id = other._id;
	_type = other._type;
}

Entry::~Entry() {
	
}

int Entry::getId() {
	return _id;
}

QString Entry::getType() {
	return _type;
}


