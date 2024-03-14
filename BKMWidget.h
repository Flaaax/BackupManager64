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
	BKMWidget(QWidget* parent = nullptr);
	~BKMWidget() { bkManager.saveAll(); delete ui;}

private slots:
	void onClick_addBkButton();
	void onClick_saveButton();
	void onClick_SPButton();								//start and pause button
	void onClick_settingsButton();
	void onClick_bk_roll_Button();
	void onChange_bkNameList(QListWidgetItem* item);		//更新namelist和currentBackup
	void showMenu_bkNameList(const QPoint& pos);
	void showMenu_allBkList(const QPoint& pos);
	void updateAll();

private:
	void initLogger();
	void setBtnIcon();
	void update_backupNameList();
	void showMessage(const QString& message);
	void update_SPButtonStat();
	void update_allBkList();
	void nameListDelete_dialog(QListWidgetItem* item);
	bool check_backupValid();
	void log(const QString& msg);
	QWidget* focusedList = QApplication::focusWidget();
	BackupManagerQt bkManager;								//包含构造函数
	Ui::BKMWidgetClass* ui = new Ui::BKMWidgetClass;
	qint64 lastClickTime = 0;
	QMutex updateMtx;

};


#endif
