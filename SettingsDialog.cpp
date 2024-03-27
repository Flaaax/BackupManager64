#include "SettingsDialog.h"
#include <qtimer.h>

SettingsDialog::SettingsDialog(QWidget* parent, const BackupConfig& currentConfigs):QDialog(parent)
{
	ui->setupUi(this);
	ui->autobkPeriodEd->installEventFilter(this);
	this->configs = currentConfigs;
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	ui->autobkPeriodEd->setText(QString::fromStdString(std::to_string(configs.autobackupPeriod)));
	ui->autobkPeriodEd->setPlaceholderText(QString::fromStdString(std::to_string(30ULL)));
	//ui->autobkEnableBtn->setChecked(configs.autobackupEnabled);
	connect(ui->okButton, &QPushButton::clicked, this, &SettingsDialog::onClick_okButton);
	connect(ui->rejectButton, &QPushButton::clicked, this, [this]() {this->reject(); });
}

BackupConfig SettingsDialog::getUserInput() const
{
	return configs;
}

void SettingsDialog::onClick_okButton()
{
	//configs.autobackupEnabled = ui->autobkEnableBtn->isChecked();
	configs.autobackupPeriod = ui->autobkPeriodEd->text().toLongLong();
	this->accept();
}

bool SettingsDialog::eventFilter(QObject* watched, QEvent* event)
{
	if (watched == ui->autobkPeriodEd && event->type() == QEvent::FocusIn) {
		QTimer::singleShot(0, ui->autobkPeriodEd, &QLineEdit::selectAll);
	}
	return QDialog::eventFilter(watched, event);
}

void SettingsDialog::onFinish_autobkPeriodEd()
{
	long long period = ui->autobkPeriodEd->text().toULongLong();
	if (period <= 0) {
		QApplication::beep();
		ui->autobkPeriodEd->setText(QString::fromStdString(std::to_string(30ULL)));
	}
	else {
		ui->autobkPeriodEd->setText(QString::fromStdString(std::to_string(period)));
		configs.autobackupPeriod = static_cast<time_t>(period);
	}
}
