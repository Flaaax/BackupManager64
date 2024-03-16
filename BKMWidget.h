#pragma once
#ifndef BKMWIDGET_H
#define BKMWIDGET_H
#include "BackupManager/BackupManagerQt.h"
#include "ui_BKMWidget.h"
#include <QPropertyAnimation>
#include <QtWidgets/QWidget>
#include <qmutex>

//todo �Ż��Ҽ��˵������������Ŀ
//todo ��Ӹ�������
//todo �����̳߳��е��쳣
//todo ʹ��qt�̳߳أ�����ֱ�ӿ���gui����Ϊ������
//todo �ڴ浵�б�������浵��Ϣ
//todo theres a problem in settings button
//todo ��ӿ��ٲ�����ȷ��ѡ��
//todo ����BackupManagerQt�е���־
//������
class BKMWidget : public QWidget
{
	Q_OBJECT
public:

	BKMWidget(QWidget* parent = nullptr);
	~BKMWidget() { bkManager.saveAll();}					//dont need to delete ui

private slots:

	void addNewBackupItem();
	void saveConfigs();
	void onClick_SPButton();								//start and pause button
	void openSettings();
	void onClick_bk_roll_Button();
	void onItemChange_bkNameList(QListWidgetItem* item);		//����namelist��currentBackup
	void callMenu_bkNameList(const QPoint& pos);
	void callMenu_allBkList(const QPoint& pos);
	void updateAll();

private:

	Ui::BKMWidgetClass* ui = new Ui::BKMWidgetClass;

	BackupManagerQt bkManager;								//�������캯��

	qint64 lastClickTime = 0;

	void initMenuBar();
	void initLogger();
	void initButtonIcon();
	void showMessage(const QString& message);
	void update_backupNameList();
	void update_SPButtonStat();
	void update_allBkList();
	void callBackupItemDeleteDialog(QListWidgetItem* item);
	bool check_backupValid();
	void log(const QString& msg);
};


#endif
