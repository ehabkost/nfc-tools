#include "Content.h"

Content::Content(int id) {
	_id = id;
	_type = "???";
	_te = new TitleEntry;
}

Content::Content(int id, QList<Entry*> qlnc)
  : Entry(id,QString("SmartPoster"))
 {
	_te = new TitleEntry;
	_entries.append(qlnc);
}

Content::Content( const Content& other ) {
	_te = new TitleEntry(*other._te);
	_id = other._id;
	_type = other._type;
	_entries.append(other._entries);
}

Content::~Content() {
	QList<Entry*>::iterator it;
	for(it=_entries.begin(); it!=_entries.end(); ++it) 
		delete *it;
	delete _te;
}

void Content::setType(QString t) {
	_type = t;
}

void Content::addEntry(Entry* en) {
	_entries.append(en);
}

int Content::getId() {
	return _id;
}

QByteArray* Content::getData() {
   QByteArray* qba = new QByteArray;
   QList<Entry*>::iterator it;
   for(it=_entries.begin();it!=_entries.end();++it) {
     qba->append( *(*it)->getData() );
   }
	return qba;
}

TitleEntry* Content::getTitleEntry() {
	return _te;
}

