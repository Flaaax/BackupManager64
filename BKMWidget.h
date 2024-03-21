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
	void onItemChange_bkNameList(QListWidgetItem* item);	//����namelist��currentBackup
	void callMenu_backupItemList(const QPoint& pos);
	void callMenu_backupFileList(const QPoint& pos);
	void refresh();

private:

	Ui::BKMWidgetClass* ui = new Ui::BKMWidgetClass;

	BackupManagerQt bkManager;								//�������캯��

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