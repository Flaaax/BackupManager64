#pragma once
#ifndef BKMWIDGET_H
#define BKMWIDGET_H
#include "BackupManager/BackupManagerQt.h"
#include "ui_BKMWidget.h"
#include <QPropertyAnimation>
#include <QtWidgets/QWidget>
#include <qmutex>

//todo 优化右键菜单，添加属性栏目
//todo 将ThreadPool嵌入BackupManagerQt类
//todo 给ThreadPool添加异常处理
//todo 添加更多配置
//todo 捕获线程池中的异常
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
	void onClick_flushBtn();
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
	QWidget* focusedList = QApplication::focusWidget();
	BackupManagerQt bkManager;								//包含构造函数
	Ui::BKMWidgetClass* ui = new Ui::BKMWidgetClass;
	qint64 lastClickTime = 0;
	QMutex updateMtx;

};


#endif
