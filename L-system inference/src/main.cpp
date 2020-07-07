#include <QtGui>
#include "window.h"

int main(int argv, char **args)
{
	QCoreApplication::addLibraryPath("./");
	QApplication app(argv, args);
	app.setApplicationName("IPML2D");
	MainWindow window;

	window.show();
	return app.exec();
}
