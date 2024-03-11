#pragma execution_character_set("utf-8")
#include "AddBkDialog.h"
#include "BKMWidget.h"
#include "SettingsDialog.h"
#include "SaveDialog.h"
#include <qboxlayout.h>
#include <qdatetime.h>
#include <qdesktopservices.h>
#include <qgraphicseffect.h>
#include <qmenu.h>
#include <QMessageBox>
#include <qtimer.h>
#include <qurl.h>
#include <algorithm>

namespace fs = std::filesystem;

static QWidget* createAllbkListItem(const std::filesystem::path& filepath)
{
	QWidget* widget = new QWidget();
	QHBoxLayout* layout = new QHBoxLayout(widget);
	static const QFont font("΢���ź�");
	//QString leftText = QString::fromStdWString(BackupConfig::getBackupName(filepath));
	QString leftText = QString::fromStdWString(filepath.filename().wstring());
	QString rightText = QString::fromStdString(BackupConfig::timeSinceModification(filepath));
	QLabel* leftLabel = new QLabel(leftText);
	QLabel* rightLabel = new QLabel(rightText);
	leftLabel->setFont(font);
	rightLabel->setFont(font);
	rightLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	layout->addWidget(leftLabel);
	layout->addWidget(rightLabel);
	layout->setSpacing(0);
	// ȷ���Ҳ�ı�ǩ���������������ÿռ�
	layout->setStretch(1, 1);
	layout->setContentsMargins(3, 3, 3, 3); // ���ý�С�ı߾�
	//layout->setSpacing(3); // ���ý�С�Ŀؼ����
	widget->setLayout(layout);

	return widget;
}

BKMWidget::BKMWidget(QWidget* parent) : QWidget(parent)
{
	ui->setupUi(this);
	this->setFixedSize(585, 440);
	std::string version = BackupConfig::loadVersion();
	QString title = QString::fromStdString("BackupManager x64 v" + version);
	this->setWindowTitle(title);
	this->setBtnIcon();
	ui->bkNameList->setSortingEnabled(false);
	ui->bkNameList->setContextMenuPolicy(Qt::CustomContextMenu);
	ui->allBkList->setContextMenuPolicy(Qt::CustomContextMenu);
	ui->bkNameList->setStyleSheet("QListWidget { background-color: rgb(250,250,250); }");
	ui->allBkList->setStyleSheet("QListWidget { background-color: rgb(250,250,250); }");
	connect(ui->addBkButton, &QPushButton::clicked, this, &BKMWidget::onClick_addBkButton);
	connect(ui->saveButton, &QPushButton::clicked, this, &BKMWidget::onClick_saveButton);
	connect(ui->settingsBtn, &QPushButton::clicked, this, &BKMWidget::onClick_settingsButton);
	connect(ui->flushBtn, &QPushButton::clicked, this, &BKMWidget::onClick_flushBtn);
	connect(ui->moveBtn_up, SIGNAL(clicked()), this, SLOT(onClick_moveButton()));
	connect(ui->moveBtn_down, SIGNAL(clicked()), this, SLOT(onClick_moveButton()));
	connect(ui->startButton, SIGNAL(clicked()), this, SLOT(onClick_SPButton()));
	connect(ui->pauseButton, SIGNAL(clicked()), this, SLOT(onClick_SPButton()));
	connect(ui->quickBkButton, SIGNAL(clicked()), this, SLOT(onClick_bk_roll_Button()));
	connect(ui->quickRollbackBtn, SIGNAL(clicked()), this, SLOT(onClick_bk_roll_Button()));
	connect(ui->saveBtn, SIGNAL(clicked()), this, SLOT(onClick_bk_roll_Button()));
	connect(ui->loadBtn, SIGNAL(clicked()), this, SLOT(onClick_bk_roll_Button()));
	connect(ui->bkNameList, &QListWidget::itemChanged, this, &BKMWidget::onChange_bkNameList);
	connect(ui->bkNameList, &QListWidget::itemChanged, this, &BKMWidget::update_allBkList);
	connect(ui->bkNameList, &QListWidget::customContextMenuRequested, this, &BKMWidget::showMenu_bkNameList);
	connect(ui->allBkList, &QListWidget::customContextMenuRequested, this, &BKMWidget::showMenu_allBkList);
	this->setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
	this->update_backupNameList();
	this->update_SPButtonStat();
	this->update_allBkList();
	//QTimer* timer = new QTimer(this);
	//connect(timer, &QTimer::timeout, this, [this]() {update_allBkList(); });
	//timer->start(30000);	//ÿ5min����һ�δ浵�б�
}

void BKMWidget::onClick_addBkButton()
{
	AddBkDialog newDialog(this);
	if (newDialog.exec() == QDialog::Accepted) {
		bkManager.addBackup(newDialog.getUserInput());
		update_backupNameList();
		showMessage("���Ӵ浵�ɹ�");
	}
}

void BKMWidget::onClick_saveButton()
{
	bkManager.saveAll();
	showMessage("����ɹ�");
}

void BKMWidget::onClick_SPButton()
{
	QObject* obj = sender();
	time_t bktime = bkManager.getAutobackupTimer();
	if (obj == ui->startButton) {
		bkManager.resumeAutoBackup();
		showMessage(QString("�Զ��浵������ time=") + QString::number(bktime));
	}
	else if (obj == ui->pauseButton) {
		bkManager.pauseAutoBackup();
		showMessage(QString("�Զ��浵������ time=") + QString::number(bktime));
	}
	update_SPButtonStat();
}

void BKMWidget::onChange_bkNameList(QListWidgetItem* item)
{
	ui->bkNameList->blockSignals(true);
	bkManager.resetAutoBackup();
	if (item->checkState() == Qt::Checked) {
		int checkedItemPos = -1;
		for (int i = 0; i < ui->bkNameList->count(); ++i) {
			QListWidgetItem* otherItem = ui->bkNameList->item(i);
			if (otherItem != item) {
				otherItem->setCheckState(Qt::Unchecked);
			}
			else {
				checkedItemPos = i;
			}
		}
		bkManager.setCurrentBackup(checkedItemPos);
	}
	else {
		bkManager.setCurrentBackup(-1);
	}
	ui->bkNameList->blockSignals(false);
}

void BKMWidget::showMenu_bkNameList(const QPoint& pos)
{
	QListWidgetItem* item = ui->bkNameList->itemAt(pos);
	int index = ui->bkNameList->row(item);
	if (item) {
		QMenu menu;
		QMenu* browseMenu = menu.addMenu(tr("���·��"));
		QAction* browseSourceAction = browseMenu->addAction(tr("���Դ�ļ�·��"));
		QAction* browseBackupAction = browseMenu->addAction(tr("����浵·��"));

		QAction* editAction = menu.addAction(tr("�༭"));
		QAction* removeAction = menu.addAction(tr("�Ƴ�"));
		QAction* deleteAction = menu.addAction(tr("ɾ���浵"));

		QAction* selectedAction = menu.exec(ui->bkNameList->viewport()->mapToGlobal(pos));
		if (selectedAction == deleteAction) {
			nameListDelete_dialog(item);
		}
		else if (selectedAction == browseSourceAction) {
			QString sourcePath = QString::fromStdString(bkManager.getBackup(index).source_path.string());
			QDesktopServices::openUrl(QUrl::fromLocalFile(sourcePath));
		}
		else if (selectedAction == browseBackupAction) {
			QString backupPath = QString::fromStdString(bkManager.getBackup(index).backup_path.string());
			QDesktopServices::openUrl(QUrl::fromLocalFile(backupPath));
		}
		else if (selectedAction == removeAction) {
			bkManager.removeBackup(index, false);
			bkManager.saveAll();
			update_backupNameList();
		}
		else if (selectedAction == editAction) {
			AddBkDialog newDialog(this);
			int res = newDialog.exec();
			if (res == QDialog::Accepted) {
				bkManager.editBackup(newDialog.getUserInput(), index);
			}
			update_backupNameList();
		}
	}
}

void BKMWidget::showMenu_allBkList(const QPoint& pos)
{
	QListWidgetItem* item = ui->allBkList->itemAt(pos);
	int index = ui->allBkList->row(item);
	auto backups = bkManager.getAllBackups();
	if (index >= 0 && index <= backups.size() - 1) {
		const auto& path = backups[index].first;
		QMenu menu;
		QAction* copyAction = menu.addAction(tr("����"));
		QAction* deleteAction = menu.addAction(tr("ɾ��"));
		QAction* browseAction = menu.addAction(tr("�鿴·��"));
		QAction* selectedAction = menu.exec(ui->allBkList->viewport()->mapToGlobal(pos));
		if (selectedAction == copyAction) {
			bkManager.copyBackup(path);
		}
		else if (selectedAction == deleteAction) {
			bkManager.deleteBackup(path);
		}
		else if (selectedAction == browseAction) {
			QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdWString(path.wstring())));
		}
		update_allBkList();
	}
}

void BKMWidget::onClick_flushBtn()
{
	update_allBkList();
	update_backupNameList();
	update_SPButtonStat();
}

void BKMWidget::onClick_moveButton()
{
	bkManager.resetAutoBackup();
	QObject* obj = sender();
	int index = ui->bkNameList->currentRow();
	int next_index = -1;
	if (index == -1) {
		return;
	}
	else if (obj == ui->moveBtn_up && index - 1 >= 0) {
		next_index = index - 1;
	}
	else if (obj == ui->moveBtn_down && index + 1 <= bkManager.getBackupListSize() - 1) {
		next_index = index + 1;
	}
	if (next_index != -1) {
		bkManager.swapBackups(index, next_index);
		update_backupNameList();
		ui->bkNameList->setCurrentRow(next_index);
	}
}

void BKMWidget::onClick_settingsButton()
{
	SettingsDialog newDialog(this, bkManager.getConfigs());
	int res = newDialog.exec();
	if (res == QDialog::Accepted) {
		bkManager.setConfigs(newDialog.getUserInput());
		showMessage("����ɹ�");
	}
}

void BKMWidget::onClick_bk_roll_Button()
{
	qint64 now = QDateTime::currentMSecsSinceEpoch();
	if (now - lastClickTime < 2000) {
		// ��������ϴε������2�룬ֱ�ӷ���
		/*
		QMessageBox msg(this);
		msg.setWindowTitle(":(");
		msg.setText("����̫����");
		msg.exec();
		*/
		QApplication::beep();
		this->showMessage("����̫����");
		return;
	}
	QObject* obj = sender();
	if (bkManager.getBackup(BackupManagerQt::CURRENT).empty()) {
		QApplication::beep();
		this->showMessage("��������Ϊ��");
		return;
	}
	if (obj == ui->quickBkButton) {
		bkManager.createBackup();
		if (bkManager.goodStatus()) {
			showMessage("�浵�ɹ�");
		}
		else {
			showMessage("�浵ʧ�ܣ�");
		}
		update_allBkList();
		lastClickTime = now;
	}
	else if (obj == ui->quickRollbackBtn) {
		BackupManagerQt::folders backups = bkManager.getAllBackups();
		if (backups.empty()) {
			showMessage("ʧ�ܣ�û�д浵");
			return;
		}
		//todo ����ص�ȷ��ѡ��
		bkManager.rollback(backups[0].first);
		if (bkManager.goodStatus()) {
			showMessage("�ص��ɹ�");
		}
		else {
			showMessage("�ص�ʧ�ܣ�");
		}
	}
	else if (obj == ui->saveBtn) {
		SaveDialog dialog(this);
		auto ret = dialog.exec();
		if (ret == QDialog::Accepted) {
			auto saveName = dialog.getUserInput();
			auto backups = bkManager.getAllBackups();
			if (std::any_of(backups.begin(), backups.end(), [saveName](auto item) {return item.first.filename() == saveName; }))
			{
				QMessageBox msg(this);
				msg.setWindowTitle("��ȷ��");
				msg.setText(QString::fromStdWString(L"Ҫ���Ǵ浵 " + saveName + L"��"));
				msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
				msg.setDefaultButton(QMessageBox::No);
				int ret = msg.exec();
				if (ret == QMessageBox::Yes) {
					bkManager.createBackup(saveName, true);
				}
				else {
					showMessage("��ȡ���浵");
				}
				return;
			}
			else {
				bkManager.createBackup(saveName);
				if (bkManager.goodStatus()) {
					showMessage("�浵�ɹ�");
				}
				else {
					showMessage("�浵ʧ�ܣ�");
				}
				update_allBkList();
			}
		}

	}
	else if (obj == ui->loadBtn) {
		BackupManagerQt::folders backups = bkManager.getAllBackups();
		if (backups.empty()) {
			showMessage("ʧ�ܣ�û�д浵");
			return;
		}
		int index = ui->allBkList->currentRow();
		//todo ����ص�ȷ��ѡ��
		showMessage("���ٻص���");
		if (index == -1)index = 0;
		if (index >= 0 && index <= backups.size() - 1) {
			QString name = QString::fromStdWString(backups[index].first.filename().wstring());
			QMessageBox msg(this);
			//msg.setMinimumSize(200, 300);
			msg.setWindowTitle("ִ�лص�");
			msg.setText("���Ҫ�ص�\n" + name + "\n��?");
			msg.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
			int res = msg.exec();
			if (res == QMessageBox::Ok) {
				bkManager.rollback(backups[index].first);
				if (bkManager.goodStatus()) {
					showMessage("�ص��ɹ�");
				}
				else {
					showMessage("�ص�ʧ�ܣ�");
				}
				update_allBkList();
				lastClickTime = now;
			}
		}
	}
	else {
		showMessage("ʧ�ܣ�δ֪����");
	}
}

void BKMWidget::setBtnIcon()
{
	ui->pauseButton->setIcon(QIcon("resources//redsquare.png"));
	ui->pauseButton->setIconSize(QSize(60, 60));
	ui->startButton->setIcon(QIcon("resources//greenarrow.png"));
	ui->pauseButton->setIconSize(QSize(60, 60));
	ui->myPicture->setIcon(QIcon("resources//wha.png"));
	ui->myPicture->setIconSize(QSize(121, 121));
	ui->moveBtn_down->setIcon(QIcon("resources//black_arrow2.png"));
	ui->moveBtn_down->setIconSize(QSize(25, 25));
	ui->moveBtn_up->setIcon(QIcon("resources//black_arrow1.png"));
	ui->moveBtn_up->setIconSize(QSize(25, 25));
	ui->settingsBtn->setIcon(QIcon("resources//settings.png"));
	ui->settingsBtn->setIconSize(QSize(26, 26));
}

void BKMWidget::update_backupNameList()
{
	//bkConfigs.currentItem = ui->bkNameList->currentRow();
	ui->bkNameList->blockSignals(true);
	ui->bkNameList->clear();
	for (int index = 0; index < bkManager.getBackupListSize(); index++) {
		QListWidgetItem* item = new QListWidgetItem(QString::fromStdWString(bkManager.getBackup(index).backup_name), ui->bkNameList);
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
		if (index == bkManager.getConfigs().currentItem) {
			item->setCheckState(Qt::Checked);
		}
		else {
			item->setCheckState(Qt::Unchecked);
		}
	}
	ui->bkNameList->blockSignals(false);
}

void BKMWidget::showMessage(const QString& message)
{
	QLabel* msg = new QLabel(message, this);
	msg->setGeometry(270, 40, 301, 21);
	QFont font;
	font.setFamily("΢���ź�");
	font.setPointSize(10);
	msg->setFont(font);
	msg->show();
	// up����
	QPropertyAnimation* riseAnimation = new QPropertyAnimation(msg, "geometry");
	riseAnimation->setDuration(1200); // ��������ʱ��
	riseAnimation->setStartValue(msg->geometry());
	riseAnimation->setEndValue(QRect(msg->x(), msg->y() - 50, msg->width(), msg->height())); // ����Ч��
	// ����͸���ȶ���
	QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(msg);
	msg->setGraphicsEffect(effect);
	QPropertyAnimation* opacityAnimation = new QPropertyAnimation(effect, "opacity");
	opacityAnimation->setDuration(1000); // ����ʱ��
	opacityAnimation->setStartValue(0.9); // ��ʼ͸����
	opacityAnimation->setEndValue(0.0); // ����͸��
	// ��������ɺ�ɾ��label
	QObject::connect(riseAnimation, &QPropertyAnimation::finished, msg, &QLabel::deleteLater);
	riseAnimation->start(QPropertyAnimation::DeleteWhenStopped);
	opacityAnimation->start(QPropertyAnimation::DeleteWhenStopped);
	//QTimer::singleShot(1200, msg, &QLabel::deleteLater);
}

void BKMWidget::update_SPButtonStat()
{
	bool isRunning = bkManager.isAutobkRunning();
	ui->startButton->setEnabled(!isRunning);
	ui->pauseButton->setEnabled(isRunning);
}

void BKMWidget::update_allBkList()
{
	ui->allBkList->blockSignals(true);
	ui->allBkList->clear();
	auto backups = bkManager.getAllBackups();
	//int currentItem = bkManager.getConfigs().currentItem;
	//QString name = QString::fromStdString(bkManager.getBackup(currentItem).backup_name);
	if (!backups.empty()) {
		for (const auto& bk : backups) {
			QListWidgetItem* item = new QListWidgetItem(ui->allBkList);
			std::string time = BackupConfig::timeSinceModification(bk.first);
			QWidget* listItemWidget = createAllbkListItem(bk.first);
			item->setSizeHint(listItemWidget->sizeHint());
			ui->allBkList->setItemWidget(item, listItemWidget);
			ui->allBkList->addItem(item);
		}
	}
	else {
		QListWidgetItem* item = new QListWidgetItem("��", ui->allBkList);
	}
	ui->allBkList->blockSignals(false);
}

bool BKMWidget::check_backupValid()
{
	if (bkManager.isBackupValid(BackupManagerQt::ALL)) {
		QMessageBox msg(this);
		msg.setWindowTitle("����");
		QString text = "�浵 " + QString::fromStdWString(bkManager.getBackup().backup_name) + " �Ҳ���·����\n����·�����޸�";
		msg.setText(text);
		QPushButton* btn = msg.addButton(tr("ȷ��"), QMessageBox::AcceptRole);
		msg.exec();
		return false;
	}
	return true;
}

void BKMWidget::nameListDelete_dialog(QListWidgetItem* item)
{
	QMessageBox msgBox(this);
	msgBox.setWindowTitle(tr("ɾ���浵"));
	QString name = item->text();
	QString text = "ѡ��\"�Ƴ�\"�Ὣ " + name + " ���б����ų�\nѡ��\"ɾ��ȫ��\"��ɾ�� " + name + " �е����д浵\n";
	msgBox.setText(text);
	QPushButton* btnDetach = msgBox.addButton(tr("�Ƴ�"), QMessageBox::AcceptRole);
	QPushButton* btnDelete = msgBox.addButton(tr("ɾ��ȫ��"), QMessageBox::DestructiveRole);
	QPushButton* btnCancel = msgBox.addButton(tr("ȡ��"), QMessageBox::RejectRole);
	msgBox.setDefaultButton(btnCancel);
	msgBox.exec();
	//auto clickedBtn = msgBox.clickedButton();
	int index = ui->bkNameList->row(item);
	if (msgBox.clickedButton() == btnDetach) {
		bkManager.removeBackup(index, false);
	}
	else if (msgBox.clickedButton() == btnDelete) {
		bkManager.removeBackup(index, true);
		QMessageBox msg;
		msg.setWindowTitle("����Ϣ");
		msg.setText("Ϊ�˷�ֹ�ֻ���û���κδ浵��ʵ��ɾ��:)");
		msg.exec();
	}
	bkManager.saveAll();
	update_backupNameList();
}