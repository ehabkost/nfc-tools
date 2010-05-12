#include "NfcUiManager.h"

#include "config.h"

NfcUiManager::NfcUiManager(QObject* parent) : QObject(parent) {
	allowNotif = false;
	lock = true;
	fullTree = false;
	//_app = app;
	(void)_qtw;
	_nfcDm = new NfcDeviceManager;
	QObject::connect(_nfcDm, SIGNAL( devicePlugged(uchar,QString) ),
		this, SLOT( fillTree() ) );
	QObject::connect(_nfcDm, SIGNAL( deviceUnplugged(uchar,QString) ),
		this, SLOT( fillTree() ) );
}

NfcUiManager::~NfcUiManager() {
	delete _qsti;
	delete _trayIconMenu;
	delete _mw;
	delete _uiMW;
	delete _nfcDm;
	/*QList<Executor*>::iterator itEx;
	for(itEx=_execs.begin();itEx!=_execs.end();++itEx) {
		delete *itEx;
	}*/
	QDir qd( QString("/tmp/desknfc-") + QString( getlogin() ) );
	QStringList content = qd.entryList();
	QStringList::iterator itS;
	for(itS=content.begin();itS!=content.end();++itS) {
		qd.remove(*itS);
	}
	qd.rmdir(qd.path());
}

QApplication* NfcUiManager::getApp() {
	return _app;
}

void NfcUiManager::run() {
  lock = false;
  allowActions = true;
  _mw = new QMainWindow;
  _uiMW = new Ui_MainWindow;
  //_mp = new MainPlasmoid(this);
  _uiMW->setupUi(_mw);
  //_uiMW->infosWidget->setPalette( _app->palette() );
  /*foreach(QObject* qw, _uiMW->infosWidget->children()) {
    QWidget* casted_qw = qobject_cast<QWidget*>(qw);
    if(casted_qw != NULL) casted_qw->setPalette( _app->palette() );
  }*/
  _qtw = _uiMW->treeWidget;
  _qsti = new QSystemTrayIcon(this);
  QIcon icon(PACKAGE_DATA_INSTALL_DIR"/icons/"PACKAGE_NAME".png");
  _qsti->setIcon(icon);
  _mw->setWindowIcon(icon);
  _trayIconMenu = new QMenu(_mw);
  _deviceManagerAction = new QAction(NfcUiManager::tr("Device Manager"), _trayIconMenu);
  _quitAction = new QAction(NfcUiManager::tr("Quit"), _trayIconMenu);
  _trayIconMenu->addAction(_deviceManagerAction);
  _trayIconMenu->addAction(_quitAction);
  QObject::connect(_deviceManagerAction,SIGNAL(triggered()),this, SLOT(showWindow()));
  //QObject::connect(_quitAction,SIGNAL(triggered()),_app,SLOT( quit() ) );
  QObject::connect(_uiMW->fullTreeCheckBox,SIGNAL(stateChanged(int)),this, SLOT(fullTreeCBHandler(int)));
  QObject::connect(_qsti,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
    this, SLOT(qstiHandler(QSystemTrayIcon::ActivationReason)));
  _qsti->setContextMenu(_trayIconMenu);
  _openAction = new QAction( NfcUiManager::tr("Open"), this );
  QObject::connect(_openAction, SIGNAL( triggered() ), this, SLOT( showWindow() ) );
  _qsti->show();
  fillTree();
}

void NfcUiManager::showContentWidget(QTreeWidgetItem* item,Content* pt_Content) {
  if(_uiIW && _uiMW->infosWidget->objectName() == "ContentWidget") 
    delete (Ui_ContentWidget*)_uiIW;
  else if(_uiIW && _uiMW->infosWidget->objectName() == "TargetWidget") 
    delete (Ui_TargetWidget*)_uiIW;
  _uiIW = NULL;
  _uiIW = new Ui_ContentWidget();
  Ui_ContentWidget* casted_qcw = (Ui_ContentWidget*)_uiIW ;
  if(casted_qcw != NULL) {
    casted_qcw->setupUi( _uiMW->infosWidget );
    //_uiMW->infosWidget->setPalette( _app->palette() );
    foreach(QObject* qw, _uiMW->infosWidget->children()) {
      QWidget* casted_qw = qobject_cast<QWidget*>(qw);
      //if(casted_qw != NULL) casted_qw->setPalette( _app->palette() );
    }
    if(! item->text(0).contains("(URI)") ) {
      QByteArray data = QByteArray().append(*(pt_Content->getData()));
      QString path = "";
      if(!_paths.contains(data)) {
        path = makeFile(data,pt_Content->getType());
        _paths.insert(data,path);
      }
      else path = _paths.value(data);
      KFileItem fileItem(KFileItem::Unknown, KFileItem::Unknown, path);
      casted_qcw->typeLabel->setText(NfcUiManager::tr("Type: ") + fileItem.mimeComment());
      casted_qcw->iconLabel->setPixmap( KIconLoader().loadIcon(fileItem.iconName(), KIconLoader::Desktop ));
    } else {
      KUrl url(*(pt_Content->getData()));
      casted_qcw->typeLabel->setText(url.protocol() );
      casted_qcw->iconLabel->setPixmap( KIconLoader().loadIcon(
			KMimeType::iconNameForUrl(url), KIconLoader::Desktop));
    }
      QObject::connect(casted_qcw->openButton,SIGNAL(clicked()),
		  this, SLOT(openBtClicked()) );
  }
}

void NfcUiManager::showTargetWidget(QTreeWidgetItem* qtwi, NfcTarget* target) {
  (void)qtwi;
  (void)target;
  if(_uiIW && _uiMW->infosWidget->objectName() == "ContentWidget") 
    delete (Ui_ContentWidget*)_uiIW;
  else if(_uiIW && _uiMW->infosWidget->objectName() == "TargetWidget") 
    delete (Ui_TargetWidget*)_uiIW;
  _uiIW = NULL;
  _uiIW = new Ui_TargetWidget();
  Ui_TargetWidget* casted_qtw = (Ui_TargetWidget*)_uiIW ;
  if(casted_qtw != NULL) {
    casted_qtw->setupUi( _uiMW->infosWidget );
    //_uiMW->infosWidget->setPalette( _app->palette() );
    foreach(QObject* qw, _uiMW->infosWidget->children()) {
      QWidget* casted_qw = qobject_cast<QWidget*>(qw);
      //if(casted_qw != NULL) casted_qw->setPalette( _app->palette() );
    }
  }
  QObject::connect(((Ui_TargetWidget*)_uiIW)->writeButton,SIGNAL(clicked()),this,SLOT(writeAFile()));
}

void NfcUiManager::showDeviceWidget(QTreeWidgetItem* qtwi) {
  (void)qtwi;
  if(_uiIW && _uiMW->infosWidget->objectName() == "ContentWidget") 
    delete (Ui_ContentWidget*)_uiIW;
  if(_uiIW && _uiMW->infosWidget->objectName() == "TargetWidget") 
    delete (Ui_TargetWidget*)_uiIW;

  _uiIW = NULL;
}

void NfcUiManager::writeAFile() {
  QList<QTreeWidgetItem*> l = _qtw->selectedItems();
	if(l.size() == 1) {
		QTreeWidgetItem* item = l.at(0);
		QList< QPair<QTreeWidgetItem*,NfcTarget*> >::iterator it;
		for(it=_targetsQtwi.begin(); it!=_targetsQtwi.end(); ++it) {
			if( item==(*it).first) {
				QString fileName = QFileDialog::getOpenFileName(_mw,
					tr("File to write"), "", tr("All (*)"));
				KFileItem fileItem(KFileItem::Unknown, KFileItem::Unknown, fileName);
				QFile f(fileName);
				f.open(QIODevice::ReadOnly);
				(*it).second->writeAFile(f.readAll(),fileItem.mimetype());
				f.close();
				break;
			}
		}
	}
}

void NfcUiManager::fullTreeCBHandler(int i) {
	if(i==0) fullTree = false;
	else fullTree = true;
	fillTree();
}

void NfcUiManager::qstiHandler(QSystemTrayIcon::ActivationReason ar) {
  if(ar==QSystemTrayIcon::DoubleClick) showWindow();
}

void NfcUiManager::showWindow() {
	_uiMW->infosWidget->setPalette(QPalette(Qt::red));
	QObject::connect(_qtw,SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
		this,SLOT(qtwiDoubleClickHandler(QTreeWidgetItem*,int)));
	QObject::connect(_qtw,SIGNAL(itemSelectionChanged()),
		this,SLOT(actualizeInfos()));
	_mw->show();
}

void NfcUiManager::removeTarget(QString uid, QString name) {
	(void)name; // now GCC will be silent.
	int size =  _targetsQtwi.size();
	for(int i=0; i < size; i++) {
		QString currentUid = _targetsQtwi.at(i).second->getUid();
		if(currentUid == uid) {
			if( _targetsQtwi.at(i).first != NULL ) {
				QTreeWidgetItem *parent = _targetsQtwi.at(i).first->parent();
				int index;
				if (parent) {
					index = parent->indexOfChild(_targetsQtwi.at(i).first);
					delete parent->takeChild(index);
				} else {
					index = _qtw->indexOfTopLevelItem(_targetsQtwi.at(i).first);
					delete _qtw->takeTopLevelItem(index);
				}
			}
			_qtw->update();
			_targetsQtwi.removeAt(i);
			i--;
			size--;
		}
	}
}

bool NfcUiManager::addTarget(NfcTarget* tg, QObject* parent) {
	bool res = false;
	if(tg) {
	bool present = false;
		if(!_targetsQtwi.empty()) {
			for(int i=0; i < _targetsQtwi.size(); ++i) {
				if( tg->getUid() == _targetsQtwi.at(i).second->getUid() ) {
					present = true;
					break;
				}
			}
		}
		if(!present) {
			QObject* parent2 = parent;
			if(fullTree) {
				QTreeWidgetItem* qtwi2 = new QTreeWidgetItem( ((QTreeWidgetItem*)parent) );
				qtwi2->setText(0 , tg->getName() + " (" + tg->getUid() + ")");
				((QTreeWidgetItem*)parent)->addChild(qtwi2);
				_targetsQtwi.append( QPair<QTreeWidgetItem*,NfcTarget*>( qtwi2,tg ) );
				parent2 = ((QObject*)qtwi2);
			}
			QStringList contentList = tg->getContentListStrings();
			QStringList::iterator it3;
			for( it3 = contentList.begin(); it3 != contentList.end(); ++it3) {
				QString type = (*it3).remove("\n");
				int contentId = (*it3).remove(QRegExp(":.*")).toInt();
				if(allowNotif) notify( tg->getContentById(contentId));
				QTreeWidgetItem* qtwi3 = new QTreeWidgetItem( ( fullTree ? (QTreeWidgetItem*)parent2 : NULL ) );
			    if(!type.contains("(URI)") ) {
   				QByteArray data = QByteArray().append(*(tg->getContentById(
						contentId)->getData()));
     				QString path = "";
    				if(!_paths.contains(data)) {
						path = makeFile(data,type);
						_paths.insert(data,path);
	      		}
					else path = _paths.value(data);

   		      KFileItem fileItem(KFileItem::Unknown, KFileItem::Unknown, path);
      		   qtwi3->setText( 0, fileItem.mimeComment() ); 
				}
				else qtwi3->setText( 0, (*it3).remove(QRegExp(".*: ")).remove("\n") );
				if(fullTree) 
					((QTreeWidgetItem*)parent2)->addChild(qtwi3);
				else _qtw->addTopLevelItem(qtwi3);
				_targetsQtwi.append( QPair<QTreeWidgetItem*,NfcTarget*>(qtwi3, tg ) );
				QPair<QTreeWidgetItem*,Content*> element(qtwi3,
				tg->getContentById((*it3).remove(QRegExp(":.*")).toInt()) );
				_contentsQtwi << element;
			}
			res = true;
		}
	}
	return res;
}

void NfcUiManager::addTarget(QString uid, QString name) {
	allowNotif = true;
	(void)name; // now GCC will be silent.
	QStringList devList = _nfcDm->getDeviceList();
	QStringList::iterator it;
	for( it = devList.begin(); it!= devList.end(); ++it ) {
		QObject* parent = _qtw;
		NfcDevice* dev = _nfcDm->getDeviceByName(*it);
		QStringList tgList = dev->getTargetList();
		QStringList::iterator it2;
		for( it2 = tgList.begin(); it2 != tgList.end(); ++it2) {
			NfcTarget* tg = dev->getTargetByUid(uid);
			for(int i =0; i <_devicesQtwi.size();i++) {
				if(_devicesQtwi.at(i).second->getName() == dev->getName()) {
					parent = (QObject*)_devicesQtwi.at(i).first;
					break;
				}
			}
			if(this->addTarget(tg,parent)) break;	
		}
	}
	allowNotif = false;
}

void NfcUiManager::fillTree() {
	foreach(QObject* qo, _uiMW->infosWidget->children()) {
		delete qo;
	}
	_devicesQtwi.clear();
	_targetsQtwi.clear();
	_contentsQtwi.clear();
	_qtw->clear();
	QStringList devList = _nfcDm->getDeviceList();
	QStringList::iterator it;
	// adding devices
	for( it = devList.begin(); it!= devList.end(); ++it ) {
		QObject* parent = _qtw;
		NfcDevice* dev = _nfcDm->getDeviceByName(*it);
		QObject::disconnect(dev,SIGNAL(targetFieldLeft(QString,QString)),
			this, SLOT(removeTarget(QString,QString)) );
		QObject::disconnect( dev, SIGNAL( targetFieldEntered(QString,QString) ),
			this, SLOT( addTarget(QString,QString) ) );
		QObject::connect(dev,SIGNAL(targetFieldLeft(QString,QString)),
			this, SLOT(removeTarget(QString,QString)) );
		QObject::connect(dev,SIGNAL(targetFieldEntered(QString,QString)),
			this, SLOT(addTarget(QString,QString)));
		QStringList tgList = dev->getTargetList();
		if(fullTree) {
			QTreeWidgetItem* qtwi = new QTreeWidgetItem(_qtw);
			_devicesQtwi.append( QPair<QTreeWidgetItem*,NfcDevice*>(qtwi,dev) );
			qtwi->setText(0, NfcUiManager::tr("NFC Reader") + " (" + *it + ")");
			_qtw->addTopLevelItem(qtwi);
			parent = ((QObject*)qtwi);
		}
		//adding devices contents
		QStringList::iterator it2;
		for( it2 = tgList.begin(); it2 != tgList.end(); ++it2) {
			NfcTarget* tg = dev->getTargetByUid(*it2);
			for(int i =0; i <_devicesQtwi.size();i++) {
				if(_devicesQtwi.at(i).second == dev) {
					parent = (QObject*)_devicesQtwi.at(i).first;
					break;
				}
			}
			
			this->addTarget(tg,parent);
		}
	}
}

void NfcUiManager::actualizeInfos() {
	foreach(QObject* qo, _uiMW->infosWidget->children()) {
		delete qo;
	}
	QList<QTreeWidgetItem*> l = _qtw->selectedItems();
	if(l.size() == 1) {
		QTreeWidgetItem* item = l.at(0);
		bool contentFound = false;
		QList< QPair<QTreeWidgetItem*,Content*> >::iterator it;
		for(it=_contentsQtwi.begin(); it!=_contentsQtwi.end(); ++it) {
			if( item==(*it).first) {
				contentFound = true;
				this->showContentWidget(item,(*it).second);
			}
		}
		QList< QPair<QTreeWidgetItem*,NfcTarget*> >::iterator itTg;
		for(itTg=_targetsQtwi.begin(); itTg!=_targetsQtwi.end(); ++itTg) {
			if( item==(*itTg).first && !contentFound) {
            this->showTargetWidget(item,(*itTg).second);
			}
		}
		QList< QPair<QTreeWidgetItem*,NfcDevice*> >::iterator itDev;
		for(itDev=_devicesQtwi.begin(); itDev!=_devicesQtwi.end(); ++itDev) {
			if( item==(*itDev).first) {
				this->showDeviceWidget(item);
			}
		}
	}
}

void NfcUiManager::notify(Content* content) {
	QByteArray qb(*(content->getData()));
	QString path = "";
	if(!_paths.contains(qb)) {
		path = makeFile(qb,content->getType());
		_paths.insert(qb,path);
	}
	else path = _paths.value(qb);
	Notification* notification = new Notification("contentAvailable",content,path);
   notification->setActions( QStringList( tr( "Open" ) ) );
	connect(notification, SIGNAL(activated(unsigned int)), notification , SLOT(open()) );
	notification->sendEvent();
}

void NfcUiManager::openBtClicked() {
	QList<QTreeWidgetItem*> l = _qtw->selectedItems();
	foreach(QTreeWidgetItem* item, l) {
		qtwiDoubleClickHandler(item,0);
	}
}

void NfcUiManager::qtwiDoubleClickHandler(QTreeWidgetItem * item, int column) {
	(void)column;
	QList< QPair<QTreeWidgetItem*,Content*> >::iterator it;
	for(it=_contentsQtwi.begin(); it!=_contentsQtwi.end(); ++it) {
		if(item==(*it).first) {
			//launch the program
			QByteArray content = QByteArray().append(*((*it).second->getData()));
			QString type = item->text(0);
         type.remove(QRegExp(".*: "));
			type = type.remove("\n");
			if(item->text(0).contains("(URI)")) {
				QDesktopServices::openUrl(QString(content));
			}
			else openFile(content, type);
			break;
		}
	}
}

QString NfcUiManager::makeFile(QByteArray qb, QString mimeType) {
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

void NfcUiManager::openFile(QByteArray qb, QString mimeType) {
	QString path = "";
	if(!_paths.contains(qb)) {
		path = makeFile(qb,mimeType);
		_paths.insert(qb,path);
	}
	else path = _paths.value(qb);
	QDesktopServices::openUrl(path);
}
