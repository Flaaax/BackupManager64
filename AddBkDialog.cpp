#pragma execution_character_set("utf-8")

#include "AddBkDialog.h"
#include <qfiledialog.h>
#include <qmessagebox.h>

namespace fs = std::filesystem;


static bool isFileNameValid(const QString& fileName)
{
	return !(fileName.length() >= 30) && !fileName.contains(QRegExp("[<>:\"/\\\\|?*.]"));
}


AddBkDialog::AddBkDialog(QWidget* parent, BackupItem item) :QDialog(parent)
{
	ui->setupUi(this);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	if (item.empty()) {
		this->setWindowTitle("添加存档");
	}
	else {
		this->setWindowTitle("编辑存档");
		ui->backupNameEdit->setText(QString::fromStdWString(item.name));
		ui->backupPathEdit->setText(QString::fromStdWString(item.backup_path.wstring()));
		ui->sourcePathEdit->setText(QString::fromStdWString(item.source_path.wstring()));
		ui->checkCompressBtn->setChecked(item.isCompress);
		ui->checkCompressBtn->setEnabled(false);
		ui->okButton->setText(tr("保存"));
	}
	ui->bkNameValid_label->setStyleSheet("QLabel { color : red; }");
	ui->bkNameValid_label->clear();
	ui->messageLabel->clear();
	connect(ui->okButton, &QPushButton::clicked, this, &AddBkDialog::onCLick_okButton);
	connect(ui->cancelButton, &QPushButton::clicked, this, [this]() {this->reject(); });
	connect(ui->browseSourceButton, &QPushButton::clicked, this, &AddBkDialog::onClick_browseSourceButton);
	connect(ui->browseBackupButton, &QPushButton::clicked, this, &AddBkDialog::onClick_browseBackupButton);
	connect(ui->backupNameEdit, &QLineEdit::editingFinished, this, &AddBkDialog::onFinish_backupNameEdit);
}

BackupItem AddBkDialog::getUserInput() const
{
	BackupItem newItem{
		std::filesystem::path(ui->sourcePathEdit->text().toStdWString()),
		std::filesystem::path(ui->backupPathEdit->text().toStdWString()),
		ui->backupNameEdit->text().toStdWString(),
		ui->checkCompressBtn->isChecked()
	};
	return newItem;
}

void AddBkDialog::onClick_browseSourceButton()
{
	QString directory = QFileDialog::getExistingDirectory(this, tr("选择源文件夹"), "C:\\");
	if (!directory.isEmpty()) {
		ui->sourcePathEdit->setText(directory);
		updateMessage();
	}
}

void AddBkDialog::onClick_browseBackupButton()
{
	QString directory = QFileDialog::getExistingDirectory(this, tr("选择存档文件夹"), "C:\\");		//QDir::homepath()
	if (!directory.isEmpty()) {
		ui->backupPathEdit->setText(directory);
		updateMessage();
	}
}

void AddBkDialog::onCLick_okButton()
{
	bool bkNameValid = onFinish_backupNameEdit();
	if (!bkNameValid) {
		QApplication::beep();
		ui->backupNameEdit->selectAll();
		ui->backupNameEdit->setFocus();
	}
	else if (ui->sourcePathEdit->text().isEmpty()
			 || ui->backupNameEdit->text().isEmpty()) {
		QApplication::beep();
	}
	else if (ui->sourcePathEdit->text() == ui->backupPathEdit->text()) {
		QApplication::beep();
		QMessageBox msg(this);
		msg.setWindowTitle(tr("WTF?"));
		msg.setText(tr("你不能自己存自己"));
		msg.exec();
	}
	else {
		this->accept();
	}
}

bool AddBkDialog::onFinish_backupNameEdit()
{
	QString backupName = ui->backupNameEdit->text();
	if (backupName.isEmpty()) {
		ui->bkNameValid_label->setText(tr("请输入名称"));
		return false;
	}
	else if (!isFileNameValid(backupName)) {
		ui->bkNameValid_label->setText(tr("名称不合法"));
		return false;
	}
	else {
		ui->bkNameValid_label->setText("");
		return true;
	}
}

void AddBkDialog::updateMessage()
{
	auto source_path = std::filesystem::u8path(ui->sourcePathEdit->text().toUtf8().constData());
	auto backup_path = std::filesystem::u8path(ui->backupPathEdit->text().toUtf8().constData());
	auto source_folder = source_path.filename();
	auto backup_folder = backup_path.filename();
	QString messageText = "将会在 "
		+ QString::fromStdWString(backup_folder.wstring())
		+ " 中创建 " + QString::fromStdWString(source_folder.wstring()) + " 的存档";
	ui->messageLabel->setText(messageText);
}
