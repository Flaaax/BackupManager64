#pragma once
#ifndef ADDBKDIALOG_H
#define ADDBKDIALOG_H
#include <QDialog>
#include "ui_AddBkDialog.h"
#include "BackupManager/BackupProcess.h"


class AddBkDialog : public QDialog
{
	Q_OBJECT
public:
	explicit AddBkDialog(QWidget* parent = nullptr, BackupItem item = BackupItem());
	BackupItem getUserInput()const;

private slots:
	void onClick_browseSourceButton();
	void onClick_browseBackupButton();
	void onCLick_okButton();
	bool onFinish_backupNameEdit();

private:
	void showMessage();
	Ui::AddBkDialogClass* ui = new Ui::AddBkDialogClass;
};


#endif

