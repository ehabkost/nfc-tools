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

/**
 * @brief getter for this title
 * @return title as a Byte Array
 */
QByteArray* TitleEntry::getData() {
	return new QByteArray;
}

/**
 * @brief add a new title
 * @param data new title as a Byte Array
 */
void TitleEntry::addTitle(QByteArray data) {
	quint8 langLength = data.at(0);
	QString lang(data.left(langLength+1).right(langLength));
	QString title(data.right(data.size()-langLength-1));
	this->setTitle(lang,title);
}

/**
 * @brief set a title for a lang
 * @param lang the lang code
 * @param title the title
 */
void TitleEntry::setTitle(QString lang,QString title) {
 	QList< QPair<QString,QString>* >::iterator it;
	for(it=_titles.begin(); it!=_titles.end(); ++it ) {
		if((*it)->first == lang) _titles.removeOne(*it);
	} 
	_titles.append( new QPair<QString,QString>(lang,title) );
}

/**
 * @brief get a localised title
 * @param lang the lang wanted
 */
QString TitleEntry::getTitle(QString lang) {
	QString title = _titles.empty() ? "Unknown" : _titles.first()->second;
	QList< QPair<QString,QString>* >::iterator it;
	for(it=_titles.begin(); it!=_titles.end(); ++it ) {
		if((*it)->first == lang) title = (*it)->second;
	}
	return title;
}

