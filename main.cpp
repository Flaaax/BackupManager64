#include "BKMWidget.h"
#include <QtWidgets/QApplication>
#include <QMessageBox>
#include <windows.h>

static bool check_runtime()
{
	// 尝试加载vcruntime140.dll或其他必要的DLL
	HMODULE hMod = LoadLibrary(TEXT("vcruntime140.dll"));
	if (hMod == NULL) {
		// DLL加载失败
		return false;
	}
	FreeLibrary(hMod);
	return true;
}

int main(int argc, char* argv[])
{
	/*
	if (!CheckRuntime()) {
		MessageBox(NULL, TEXT("未检测到Visual C++运行时。请安装适当的Visual C++ Redistributable。"), TEXT("运行时错误"), MB_OK | MB_ICONERROR);
		return -1;
	}
	*/
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
