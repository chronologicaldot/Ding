#include "app.h"

#ifdef USE_QT
#include <QApplication>
#endif

int main( int argc, char*  argv[] )
{
#ifdef USE_QT
	//Q_INIT_RESOURCE(qtapp);

	QApplication  qapp(argc, argv);
	App  window;
	window.show();
	return qapp.exec();
#else
	App  app;
	app.run();
	return 0;
#endif
}
