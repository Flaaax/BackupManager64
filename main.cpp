#include "BKMWidget.h"
#include <QtWidgets/QApplication>
#include <QMessageBox>


int main(int argc, char* argv[])
{
	try {
		QApplication a(argc, argv);
		//QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
		BKMWidget w;
		w.show();
		return a.exec();
	}
	catch (const std::exception& e) {
		QMessageBox::critical(nullptr, "Exception Caught", e.what());
		return -1;
	}
	catch (...) {
		QMessageBox::critical(nullptr, "Exception Caught", "An unknown exception has occurred.");
		return -1;
	}
	return 0;
}
