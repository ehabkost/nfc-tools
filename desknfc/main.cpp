#include <QTranslator>
#include <QLibraryInfo>
#include <QDebug>

#include "ui_MainWindow.h"
#include "NfcUiManager.h"

#include "config.h"

int main(int argc, char* argv[]) {
	// Q_INIT_RESOURCE(systray);
	QApplication app(argc,argv);
	QTranslator qtTranslator;
	qtTranslator.load("qt_" + QLocale::system().name(),
		QLibraryInfo::location(QLibraryInfo::TranslationsPath));
		app.installTranslator(&qtTranslator);
	QTranslator myappTranslator;
	myappTranslator.load(PACKAGE_DATA_INSTALL_DIR"/i18n/" + QLocale::system().name());
	app.installTranslator(&myappTranslator);
	NfcUiManager nfcUt(&app);
	//mw.setParent(&nfcUt);
	//ui.moveToThread(&nfcUt);
	nfcUt.run();
	return app.exec();
}
