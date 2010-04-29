#ifndef NFCUIMANAGER_H
#define NFCUIMANAGER_H

#include <QTreeWidget>
#include <QApplication>
#include <QMainWindow>
#include <QObject>
#include <QDir>
#include <QSystemTrayIcon>
#include <QDesktopServices>
#include <KUrl>
#include <KNotification>
#include <QMenu>
#include <kiconloader.h>
#include <kfileitem.h>
#include "NfcDevice.h"
#include "NfcDeviceManager.h"
#include "NfcTarget.h"
#include "NotifyInterface.h"
#include "ui_MainWindow.h"
#include "ui_ContentWidget.h"
#include "ui_TargetWidget.h"
#include "Notification.h"
#include "NotifyAction.h"

///  NfcUiManager 
/**
  * This class handle the GUI elements 
  */

class NfcUiManager : public QObject {

	Q_OBJECT

	public:
		NfcUiManager(QApplication*);
		~NfcUiManager();
		QApplication* getApp();
		bool lock;

	protected:
		QApplication* _app;
		QAction* _openAction;
		OrgFreedesktopNotificationsInterface* _ifaceNotif;
		QTreeWidget* _qtw;
		QMainWindow* _mw;
		Ui_MainWindow* _uiMW;
		void* _uiIW;
		QSystemTrayIcon* _qsti;
		QMenu* _trayIconMenu;
		QAction* _quitAction;
		QAction* _deviceManagerAction;
		//QList<Executor*> _execs;
		//QStringList _filesPath;
		QList< QPair<QTreeWidgetItem*,NfcDevice*> > _devicesQtwi;
		QList< QPair<QTreeWidgetItem*,NfcTarget*> > _targetsQtwi;
		QList< QPair<QTreeWidgetItem*,Content*> > _contentsQtwi;
		NfcDeviceManager* _nfcDm;
		QHash<QByteArray,QString> _paths;
		//QThread* _appThread;
		//QList<OrgNfc_toolsNfcdNfcDeviceInterface*> _nfcDevIntList;
		//QList<OrgNfc_toolsNfcdNfcTargetInterface*> _nfcTgIntList;
		

	public slots:
		void fullTreeCBHandler(int);
		void fillTree();
		void run();
		void showWindow();
		void addTarget(QString, QString);
		bool addTarget(NfcTarget*,QObject*);
		void removeTarget(QString,QString);
		void actualizeInfos();
		void qtwiDoubleClickHandler(QTreeWidgetItem*,int);
		void openBtClicked();
		void writeAFile();
		void qstiHandler(QSystemTrayIcon::ActivationReason);
		void notify(int,QString,bool);

	private:
		QString makeFile(QByteArray,QString);
		void showContentWidget(QTreeWidgetItem*,Content*);
		void showTargetWidget(QTreeWidgetItem*,NfcTarget*);
		void showDeviceWidget(QTreeWidgetItem*);
		void openFile(QByteArray,QString);
		bool fullTree;
		bool allowNotif;
		bool allowActions;

};

#endif //NFCUIMANAGER_H
