#include "SaveDialog.h"
#include "BackupManager/BackupConfig.h"

SaveDialog::SaveDialog(QWidget* parent, const QString& defaultText) {
	ui->setupUi(this);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	ui->saveNameEdit->setFocus();
	ui->saveNameEdit->setText(defaultText == "" ? QString::fromStdWString(ConfigHelper::getTimeStringForBackup()) : defaultText);
	ui->saveNameEdit->selectAll();
	ui->okButton->setDefault(true);
}

std::wstring SaveDialog::getUserInput() {
	return ui->saveNameEdit->text().toStdWString();
}
