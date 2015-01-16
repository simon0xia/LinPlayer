#include "linplayer.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	LinPlayer w;
	w.show();
	return a.exec();
}
