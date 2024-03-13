#pragma execution_character_set("utf-8")

#include "AddBkDialog.h"
#include <qfiledialog.h>
#include <qmessagebox.h>

namespace fs = std::filesystem;
static const QRegExp invalidCharsRegExp("[<>:\"/\\\\|?*.]");

/*
static bool containsNonAscii(const QString& string)
{
	for (QChar ch : string) {
		if (ch.unicode() > 127) {
			return true;
		}
	}
	return false;
}
*/

static bool isFileNameValid(const QString& fileName)
{
	if (fileName.length() >= 10) {
		return false;
	}
	if (fileName.contains(invalidCharsRegExp)) {
		return false;
	}
	//if (containsNonAscii(fileName)) {
	//	return false;
	//}
	return true;
}


AddBkDialog::AddBkDialog(QWidget* parent, BackupItem item)
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
		// 使用.toStdWString()确保路径字符串是宽字符格式
		std::filesystem::path(ui->sourcePathEdit->text().toStdWString()),
		std::filesystem::path(ui->backupPathEdit->text().toStdWString()),
		// 如果备份名不是路径，可以使用.toStdString()，否则也应使用.toStdWString()
		ui->backupNameEdit->text().toStdWString()
	};
	return newItem;
}

void AddBkDialog::onClick_browseSourceButton()
{
	QString directory = QFileDialog::getExistingDirectory(this, tr("选择源文件路径"), "C:\\");
	if (!directory.isEmpty()) {
		ui->sourcePathEdit->setText(directory);
		showMessage();
	}
}

void AddBkDialog::onClick_browseBackupButton()
{
	QString directory = QFileDialog::getExistingDirectory(this, tr("选择存档路径"), "C:\\");		//QDir::homepath()
	if (!directory.isEmpty()) {
		ui->backupPathEdit->setText(directory);
		showMessage();
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

void AddBkDialog::showMessage()
{
	auto source_path = std::filesystem::u8path(ui->sourcePathEdit->text().toUtf8().constData());
	auto backup_path = std::filesystem::u8path(ui->backupPathEdit->text().toUtf8().constData());
	auto source_folder = source_path.filename();
	auto backup_folder = backup_path.filename();
	QString messageText = "将会在 "
		+ QString::fromUtf8(reinterpret_cast<const char*>(backup_folder.u8string().c_str()))
		+ " 中创建 " + QString::fromUtf8(reinterpret_cast<const char*>(source_folder.u8string().c_str())) + " 的存档";
	ui->messageLabel->setText(messageText);
}
