#include "BKMWidget.h"
#include <QtWidgets/QApplication>
#include <QMessageBox>


int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	try {
		//QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
		BKMWidget w;
		w.show();
		return a.exec();
	}
	catch (const std::exception& e) {
		QMessageBox::critical(nullptr, "非正常退出", e.what());
		return -1;
	}
	catch (...) {
		QMessageBox::critical(nullptr, "Exception Caught", "An unknown exception has occurred.");
		return -1;
	}
	return 0;
}
