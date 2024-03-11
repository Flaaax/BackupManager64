#pragma once
#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H
#include "ui_SettingsDialog.h"
#include "BackupManager/Backupconfig.h"
#include <qdialog.h>

class SettingsDialog :  public QDialog
{
	Q_OBJECT

public:
	explicit SettingsDialog(QWidget* parent = nullptr, const BKConfigs& currentConfigs = BKConfigs());
	~SettingsDialog() {}
	BKConfigs getUserInput() const;

private slots:
	void onFinish_autobkPeriodEd();
	void onClick_okButton();

private:
	BKConfigs configs;
	Ui::SettingsDialogClass* ui = new Ui::SettingsDialogClass;
};

#endif
