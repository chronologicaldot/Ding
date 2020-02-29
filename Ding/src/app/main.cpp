// (c) 2019 Nicolaus Anderson

#include "app_window.h"
#include <QApplication>
#include <QString>

int main( int argc, char*  argv[] )
{
	Q_INIT_RESOURCE(qtapp);

	QApplication  qapp(argc, argv);
	QCoreApplication::setOrganizationName("chronologicaldot");
	QCoreApplication::setApplicationName("Ding");
	QCoreApplication::setApplicationVersion(QString("%1").arg(APP_VERSION));
	//AppWindow window(argc, argv);
	AppWindow  window;
	window.show();
	return qapp.exec();
}
