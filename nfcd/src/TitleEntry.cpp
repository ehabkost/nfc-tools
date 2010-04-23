#include "TitleEntry.h"

TitleEntry::TitleEntry() {
	
}

TitleEntry::TitleEntry( const TitleEntry& other) {
	QList< QPair<QString,QString>* >::const_iterator it;
	for(it=other._titles.begin(); it!=other._titles.end(); ++it){
		_titles.append(new QPair<QString,QString>((*it)->first,(*it)->second));
	}
}

TitleEntry::~TitleEntry() {
	QList< QPair<QString,QString>* >::iterator it;
	for(it=_titles.begin(); it!=_titles.end(); ++it ) {
		delete *it;
	}
}

QByteArray* TitleEntry::getData() {
	return new QByteArray;
}

void TitleEntry::addTitle(QByteArray data) {
	quint8 langLength = data.at(0);
	QString lang(data.left(langLength+1).right(langLength));
	QString title(data.right(data.size()-langLength-1));
	this->setTitle(lang,title);
}

void TitleEntry::setTitle(QString lang,QString title) {
 	QList< QPair<QString,QString>* >::iterator it;
	for(it=_titles.begin(); it!=_titles.end(); ++it ) {
		if((*it)->first == lang) _titles.removeOne(*it);
	} 
	_titles.append( new QPair<QString,QString>(lang,title) );
}

QString TitleEntry::getTitle(QString lang) {
	QString title = "Unknown";
	QList< QPair<QString,QString>* >::iterator it;
	for(it=_titles.begin(); it!=_titles.end(); ++it ) {
		if((*it)->first == lang) title = (*it)->second;
	}
	return title;
}

