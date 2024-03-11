#include "BKMWidget.h"
#include <QtWidgets/QApplication>
//#include <qtextcodec.h>

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	//QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
	BKMWidget w;
	w.show();
	return a.exec();
	system("pause");
	return 0;
}
