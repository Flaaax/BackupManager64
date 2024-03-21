#include "BKMWidget.h"
#include <QtWidgets/QApplication>
#include <QMessageBox>
#include <windows.h>

static bool check_runtime()
{
	// ���Լ���vcruntime140.dll��������Ҫ��DLL
	HMODULE hMod = LoadLibrary(TEXT("vcruntime140.dll"));
	if (hMod == NULL) {
		// DLL����ʧ��
		return false;
	}
	FreeLibrary(hMod);
	return true;
}

int main(int argc, char* argv[])
{
	/*
	if (!CheckRuntime()) {
		MessageBox(NULL, TEXT("δ��⵽Visual C++����ʱ���밲װ�ʵ���Visual C++ Redistributable��"), TEXT("����ʱ����"), MB_OK | MB_ICONERROR);
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
		QMessageBox::critical(nullptr, "�������˳�", e.what());
		return -1;
	}
	catch (...) {
		QMessageBox::critical(nullptr, "Exception Caught", "An unknown exception has occurred.");
		return -1;
	}
	return 0;
}
