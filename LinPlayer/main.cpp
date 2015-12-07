#include "linplayer.h"
#include <QtWidgets/QApplication>
#include "QTextCodec"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	LinPlayer w;
	w.resize(1024, 576);
	w.show();
	return a.exec();
}
