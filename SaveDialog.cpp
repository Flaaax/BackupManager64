#include "SaveDialog.h"
#include "BackupManager/BackupConfig.h"

SaveDialog::SaveDialog(QWidget* parent)
{
	ui->setupUi(this);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	ui->saveNameEdit->setFocus();
	ui->saveNameEdit->setText(QString::fromStdWString(ConfigHelper::getCurTimeForBackup()));
	ui->saveNameEdit->selectAll();
	ui->okButton->setDefault(true);
}

std::wstring SaveDialog::getUserInput()
{
	return ui->saveNameEdit->text().toStdWString();
}
