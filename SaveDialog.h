#pragma once
#ifndef SAVEDIALOG_H
#define SAVEDIALOG_H
#include<QDialog>
#include<ui_SaveDialog.h>


class SaveDialog :public QDialog {
	Q_OBJECT
public:
	SaveDialog(QWidget* parent = NULL, const QString& defaultText = "");
	std::wstring getUserInput();

private slots:

private:
	Ui::SaveDialogClass* ui = new Ui::SaveDialogClass;
};

#endif