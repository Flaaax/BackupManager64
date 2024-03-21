#pragma once
#ifndef BKMWIDGET_H
#define BKMWIDGET_H
#include "BackupManager/BackupManagerQt.h"
#include "ui_BKMWidget.h"
#include <QPropertyAnimation>
#include <QtWidgets/QWidget>
#include <qmutex>

//todo 优化右键菜单，添加属性栏目
//todo 添加更多配置
//todo 捕获线程池中的异常
//todo 使用qt线程池，不能直接控制gui，改为发射信
//todo 在存档列表加入更多存档信息
//todo theres a problem in settings button
//todo 添加快速操作的确认选项
//todo 补充BackupManagerQt中的日志
//主界面
class BKMWidget : public QWidget
{
	Q_OBJECT
public:
	enum Action {
		QS,
		S,
		QL,
		L,
		UNKNOWN
	};

	BKMWidget(QWidget* parent = nullptr);
	~BKMWidget() { bkManager.saveAll();}					//dont need to delete ui

private slots:

	void addNewBackupItem();
	void saveConfigs();
	void handeAutoSave();								//start and pause button
	void openSettings();
	void handleQSQL(Action action);
	void onItemChange_bkNameList(QListWidgetItem* item);	//更新namelist和currentBackup
	void callMenu_backupItemList(const QPoint& pos);
	void callMenu_backupFileList(const QPoint& pos);
	void refresh();

private:

	Ui::BKMWidgetClass* ui = new Ui::BKMWidgetClass;

	BackupManagerQt bkManager;								//包含构造函数

	void initMenuBar();
	void initLogger();
	void initButtonIcon();
	void showMsg(const QString& message);
	void update_autoSaveBtnStat();
	void update_backupItemList();
	void update_backupFileList();
	void callBackupItemDeleteDialog(QListWidgetItem* item);
	bool check_backupValid();
	void log(const QString& msg);
};


#endif