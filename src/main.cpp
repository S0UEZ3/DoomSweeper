#include "include/Constants.h"
#include <include/mainwindow.h>

#include <QApplication>
#include <QDir>
#include <QLocale>
#include <QSettings>
#include <QTranslator>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	a.setOrganizationName(SPR::ORG);
	a.setOrganizationDomain(SPR::ORG);
	a.setApplicationName(SPR::APP);

	bool debugMode = false;
	if (argc > 1 && QString(argv[1]) == "-dbg")
	{
		debugMode = true;
	}

	SPR::MainWindow window(debugMode);
	window.show();
	window.updateView();
	return a.exec();
}
