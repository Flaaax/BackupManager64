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
	~BKMWidget() { bkManager.saveAll(); delete ui;}

private slots:
	void onClick_addBkButton();
	void onClick_saveButton();
	void onClick_SPButton();								//start and pause button
	void onClick_settingsButton();
	void onClick_bk_roll_Button();
	void onChange_bkNameList(QListWidgetItem* item);		//����namelist��currentBackup
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
	BackupManagerQt bkManager;								//�������캯��
	Ui::BKMWidgetClass* ui = new Ui::BKMWidgetClass;
	qint64 lastClickTime = 0;
	QMutex updateMtx;

};


#endif
