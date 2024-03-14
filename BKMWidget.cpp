#pragma execution_character_set("utf-8")
#include "AddBkDialog.h"
#include "BKMWidget.h"
#include "Logger.h"
#include "SaveDialog.h"
#include "SettingsDialog.h"
#include "VectorTool.h"
#include <algorithm>
#include <qboxlayout.h>
#include <qdatetime.h>
#include <qdebug.h>
#include <qdesktopservices.h>
#include <qgraphicseffect.h>
#include <qmenu.h>
#include <QMessageBox>
#include <qtextdocument.h>
#include <qtimer.h>
#include <qurl.h>
#include <qmenubar.h>


namespace fs = std::filesystem;

static QWidget* createAllbkListItem(const std::filesystem::path& filepath)
{
	QWidget* widget = new QWidget();
	QHBoxLayout* layout = new QHBoxLayout(widget);
	static const QFont font("΢���ź�");
	//QString leftText = QString::fromStdWString(ConfigHelper::getBackupName(filepath));
	QString leftText = QString::fromStdWString(filepath.filename().wstring());
	QString rightText = QString::fromStdString(ConfigHelper::timeSinceModification(filepath));
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
	this->bkManager.setGeneralCallback([this](bool stat) {Logger::instance().update(stat); });
	this->initLogger();
	this->setFixedSize(585, 440);
	std::string version = ConfigHelper::loadVersion();
	QString title = QString::fromStdString("BackupManager x64 v" + version);
	this->setWindowTitle(title);
	this->setBtnIcon();
	ui->bkNameList->setSortingEnabled(false);
	ui->bkNameList->setContextMenuPolicy(Qt::CustomContextMenu);
	ui->allBkList->setContextMenuPolicy(Qt::CustomContextMenu);
	ui->bkNameList->setStyleSheet("QListWidget { background-color: rgb(250,250,250); }");
	ui->allBkList->setStyleSheet("QListWidget { background-color: rgb(250,250,250); }");
	ui->loggerEdit->setStyleSheet("QListWidget { background-color: rgb(244,244,244); }");
	connect(ui->addBkButton, &QPushButton::clicked, this, &BKMWidget::onClick_addBkButton);
	connect(ui->saveButton, &QPushButton::clicked, this, &BKMWidget::onClick_saveButton);
	connect(ui->settingsBtn, &QPushButton::clicked, this, &BKMWidget::onClick_settingsButton);
	connect(ui->flushBtn, &QPushButton::clicked, this, &BKMWidget::updateAll);
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

	this->initMenuBar();

	this->updateAll();
}

void BKMWidget::onClick_addBkButton()
{
	AddBkDialog newDialog(this);
	if (newDialog.exec() == QDialog::Accepted) {
		bkManager.addBackup(newDialog.getUserInput());
		update_backupNameList();
		showMessage("��Ӵ浵�ɹ�");
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
	time_t bktime = bkManager.getAutoBackupTimer();
	if (obj == ui->startButton) {
		bkManager.startAutoBackup();
		showMessage(QString("�Զ��浵������ time=") + QString::number(bktime));
	}
	else if (obj == ui->pauseButton) {
		bkManager.stopAutoBackup();
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
		bkManager.setBackup(checkedItemPos);
	}
	else {
		bkManager.setBackup(-1);
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
		QAction* browseSourceAction = browseMenu->addAction(tr("Դ�ļ�·��"));
		QAction* browseBackupAction = browseMenu->addAction(tr("�浵·��"));

		QAction* editAction = menu.addAction(tr("�༭"));

		QMenu* deleteMenu = menu.addMenu(tr("�Ƴ�"));
		QAction* removeAction = deleteMenu->addAction(tr("�Ƴ�"));
		QAction* deleteAction = deleteMenu->addAction(tr("ɾ��"));

		QMenu* moveMenu = menu.addMenu(tr("�ƶ�"));
		QAction* moveUpAction = moveMenu->addAction(tr("����"));
		QAction* moveDownAction = moveMenu->addAction(tr("����"));

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
			bkManager.removeBackupItem(index, false);
			bkManager.saveAll();
			update_backupNameList();
		}
		else if (selectedAction == editAction) {
			AddBkDialog newDialog(this, bkManager.getBackup(index));
			int res = newDialog.exec();
			if (res == QDialog::Accepted) {
				bkManager.editBackup(newDialog.getUserInput(), index);
			}
			update_backupNameList();
		}
		else if (selectedAction == moveUpAction || selectedAction == moveDownAction) {
			int index = ui->bkNameList->currentRow();
			int next_index = -1;
			if (index == -1) {
				return;
			}
			else if (selectedAction == moveUpAction && index - 1 >= 0) {
				next_index = index - 1;
			}
			else if (selectedAction == moveDownAction && static_cast<unsigned long long>(index) + 1 <= bkManager.getSize() - 1) {
				next_index = index + 1;
			}
			if (next_index != -1) {
				bkManager.swapBackups(index, next_index);
				update_backupNameList();
				ui->bkNameList->setCurrentRow(next_index);
			}
		}
	}
}

void BKMWidget::showMenu_allBkList(const QPoint& pos)
{
	QListWidgetItem* item = ui->allBkList->itemAt(pos);
	int index = ui->allBkList->row(item);
	auto backupFiles = bkManager.getAllBackups();
	if (VectorTool::isValid(backupFiles, index)) {
		QMenu menu;
		QAction* copyAction = menu.addAction(tr("����"));
		QAction* deleteAction = menu.addAction(tr("ɾ��"));
		QAction* browseAction = menu.addAction(tr("�鿴·��"));
		QAction* selectedAction = menu.exec(ui->allBkList->viewport()->mapToGlobal(pos));
		if (selectedAction == copyAction) {
			bkManager.copyBackup(index);
		}
		else if (selectedAction == deleteAction) {
			bkManager.deleteBackup(index);
		}
		else if (selectedAction == browseAction) {
			const auto* item = VectorTool::safeGet(backupFiles, index);
			if (item != nullptr) {
				QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdWString(item->first.wstring())));
			}
		}
		update_allBkList();
	}
}

void BKMWidget::updateAll()
{
	this->updateMtx.lock();
	update_allBkList();
	update_backupNameList();
	update_SPButtonStat();
	this->updateMtx.unlock();
}

void BKMWidget::onClick_settingsButton()
{
	SettingsDialog newDialog(this, bkManager.getConfigs());
	int res = newDialog.exec();
	if (res == QDialog::Accepted) {
		bkManager.updateConfigs(newDialog.getUserInput());
		bkManager.resetAutoBackup();
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
		bkManager.createBackup(L"", false);
		lastClickTime = now;
		showMessage("���������");
	}
	else if (obj == ui->quickRollbackBtn) {
		bkManager.rollBack(0);
		showMessage("���������");
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
				bkManager.createBackup(saveName, ret == QMessageBox::Yes);
				showMessage("���������");
			}
			else {
				bkManager.createBackup(saveName);
				showMessage("���������");
			}
		}

	}
	else if (obj == ui->loadBtn) {
		auto backups = bkManager.getAllBackups();
		if (backups.empty()) {
			showMessage("ʧ�ܣ��浵�б�Ϊ��");
			return;
		}
		int index = ui->allBkList->currentRow();
		//todo ����ص�ȷ��ѡ��
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
				bkManager.rollBack(index);
				lastClickTime = now;
			}
			showMessage("���������");
		}
	}
	else {
		showMessage("ʧ�ܣ�δ֪����");
	}
}

void BKMWidget::initMenuBar()
{
	//todo there's a lot to do LOL
	//todo maybe add a rename option
	QVBoxLayout* menuLayout = new QVBoxLayout(this);
	QMenuBar* menuBar = new QMenuBar(this);
	menuLayout->setMenuBar(menuBar);

	auto fileMenu=menuBar->addMenu("File");
	auto newBackupItemAction = fileMenu->addAction("Add new");
	auto deleteBackupItemAction = fileMenu->addAction("Delete");
	auto compressMenu = fileMenu->addMenu("Compress/Decompress");
	auto compressAction = compressMenu->addAction("Compress");
	auto decompressAction = compressMenu->addAction("Decompress");
	//auto saveConfigAction = fileMenu->addAction("Save");
	auto settingsAction = fileMenu->addAction("Settings");
	connect(fileMenu->addAction("Quit"), &QAction::triggered, [this]() {
		//add context
	});

	auto editMenu = menuBar->addMenu("Backup");
	auto backupAction = editMenu->addAction("Create");
	auto rollbackAction = editMenu->addAction("Rollback");
	auto deleteBackupAction = editMenu->addAction("Delete");
	auto copyAction = editMenu->addAction("Copy");
	auto autoMenu = editMenu->addMenu("Auto");
	auto startAutoAction = autoMenu->addMenu("Start Auto Backup");
	auto stopAutoAction = autoMenu->addMenu("Stop Auto Backup");
	auto resetAutoAction = autoMenu->addMenu("Reset Auto Backup");

	auto viewMenu = menuBar->addMenu("View");
	auto refreshAction = viewMenu->addAction("Refresh");

	auto settingsMenu = menuBar->addMenu("Settings");
	auto preferencesAction = settingsMenu->addAction("Preferences");

	auto helpMenu = menuBar->addMenu("Help");
	auto helpAction = helpMenu->addAction("View Help");
	auto aboutAction = helpMenu->addAction("About");

}

void BKMWidget::initLogger()
{
	ui->loggerEdit->setReadOnly(true);
	ui->loggerEdit->setFont(QFont("Consolas", 8));
	connect(&Logger::instance(), &Logger::messageRequest, this, [&](const QString& msg) {
		this->log(msg);
		int lineCount = ui->loggerEdit->document()->lineCount();
		if (lineCount > 1000) {
			QTextCursor cursor(ui->loggerEdit->document());
			cursor.movePosition(QTextCursor::Start);
			for (int i = 0; i < lineCount - 1000; ++i) {
				cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
				cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor);
				cursor.removeSelectedText();
				cursor.deleteChar();
			}
		}

	});
	connect(&Logger::instance(), &Logger::updateRequest, this, &BKMWidget::updateAll);
	Logger::instance().installWidgetLogger();
	Logger::debug("loggerEdit successfully set");
}

void BKMWidget::setBtnIcon()
{
	ui->pauseButton->setIcon(QIcon("resources//redsquare.png"));
	ui->pauseButton->setIconSize(QSize(60, 60));
	ui->startButton->setIcon(QIcon("resources//greenarrow.png"));
	ui->pauseButton->setIconSize(QSize(60, 60));
	ui->myPicture->setIcon(QIcon("resources//wha.png"));
	ui->myPicture->setIconSize(QSize(121, 121));
	/*
	ui->moveBtn_down->setIcon(QIcon("resources//black_arrow2.png"));
	ui->moveBtn_down->setIconSize(QSize(25, 25));
	ui->moveBtn_up->setIcon(QIcon("resources//black_arrow1.png"));
	ui->moveBtn_up->setIconSize(QSize(25, 25));
	*/
	ui->settingsBtn->setIcon(QIcon("resources//settings.png"));
	ui->settingsBtn->setIconSize(QSize(26, 26));
}

void BKMWidget::update_backupNameList()
{
	ui->bkNameList->blockSignals(true);
	ui->bkNameList->clear();
	for (int index = 0; index < bkManager.getSize(); index++) {
		QListWidgetItem* item = new QListWidgetItem(QString::fromStdWString(bkManager.getBackup(index).name), ui->bkNameList);
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
	//loggerEdit::append(message);
}

void BKMWidget::update_SPButtonStat()
{
	bool isRunning = bkManager.isAutoBackupRunning();
	ui->startButton->setEnabled(!isRunning);
	ui->pauseButton->setEnabled(isRunning);
}

void BKMWidget::update_allBkList()
{
	ui->allBkList->blockSignals(true);
	ui->allBkList->clear();
	auto backups = bkManager.getAllBackups();
	//int currentItem = bkManager.getConfigs().currentItem;
	//QString name = QString::fromStdString(bkManager.getBackup(currentItem).name);
	if (!backups.empty()) {
		for (const auto& bk : backups) {
			QListWidgetItem* item = new QListWidgetItem(ui->allBkList);
			std::string time = ConfigHelper::timeSinceModification(bk.first);
			QWidget* listItemWidget = createAllbkListItem(bk.first);
			item->setSizeHint(listItemWidget->sizeHint());
			ui->allBkList->setItemWidget(item, listItemWidget);
			ui->allBkList->addItem(item);
		}
	}
	else {
		QListWidgetItem* item = new QListWidgetItem("<��>", ui->allBkList);
	}
	ui->allBkList->blockSignals(false);
}

bool BKMWidget::check_backupValid()
{
	if (bkManager.isBackupValid(BackupManagerQt::ALL)) {
		QMessageBox msg(this);
		msg.setWindowTitle("����");
		QString text = "�浵 " + QString::fromStdWString(bkManager.getBackup().name) + " �Ҳ���·����\n����·�����޸�";
		msg.setText(text);
		QPushButton* btn = msg.addButton(tr("ȷ��"), QMessageBox::AcceptRole);
		msg.exec();
		return false;
	}
	return true;
}

void BKMWidget::log(const QString& msg)
{
	this->ui->loggerEdit->appendPlainText(msg);
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
		bkManager.removeBackupItem(index, false);
	}
	else if (msgBox.clickedButton() == btnDelete) {
		bkManager.removeBackupItem(index, true);
		QMessageBox msg;
		msg.setWindowTitle("����Ϣ");
		msg.setText("Ϊ�˷�ֹ�ֻ���û���κδ浵��ʵ��ɾ��:)");
		msg.exec();
	}
	bkManager.saveAll();
	update_backupNameList();
}