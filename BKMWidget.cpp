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
#include "QWidgetTool.h"


namespace fs = std::filesystem;

static QWidget* createAllbkListItem(const std::filesystem::path& filepath)
{
	QWidget* widget = new QWidget();
	QHBoxLayout* layout = new QHBoxLayout(widget);
	static const QFont font("微软雅黑");
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
	// 确保右侧的标签可以拉伸以填充可用空间
	layout->setStretch(1, 1);
	layout->setContentsMargins(3, 3, 3, 3); // 设置较小的边距
	//layout->setSpacing(3); // 设置较小的控件间距
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
	this->initButtonIcon();
	ui->backupItemList->setSortingEnabled(false);
	ui->backupItemList->setContextMenuPolicy(Qt::CustomContextMenu);
	ui->backupFileList->setContextMenuPolicy(Qt::CustomContextMenu);
	ui->backupItemList->setStyleSheet("QListWidget { background-color: rgb(250,250,250); }");
	ui->backupFileList->setStyleSheet("QListWidget { background-color: rgb(250,250,250); }");
	ui->loggerEdit->setStyleSheet("QListWidget { background-color: rgb(244,244,244); }");
	//connect(ui->addBkButton, &QPushButton::clicked, this, &BKMWidget::addNewBackupItem);
	//connect(ui->saveButton, &QPushButton::clicked, this, &BKMWidget::saveConfigs);
	//connect(ui->settingsBtn, &QPushButton::clicked, this, &BKMWidget::openSettings);
	//connect(ui->flushBtn, &QPushButton::clicked, this, &BKMWidget::refresh);
	connect(ui->autoSaveBtn, SIGNAL(clicked()), this, SLOT(handeAutoSave()));
	//connect(ui->pauseButton, SIGNAL(clicked()), this, SLOT(handeAutoSave()));
	connect(ui->quickSaveBtn, &QPushButton::clicked, this, [this] {handleQSQL(Action::QS); });
	connect(ui->quickLoadBtn, &QPushButton::clicked, this, [this] {handleQSQL(Action::QL); });
	connect(ui->saveBtn, &QPushButton::clicked, this, [this] {handleQSQL(Action::S); });
	connect(ui->loadBtn, &QPushButton::clicked, this, [this] {handleQSQL(Action::L); });
	connect(ui->backupItemList, &QListWidget::itemChanged, this, &BKMWidget::onItemChange_bkNameList);
	connect(ui->backupItemList, &QListWidget::itemChanged, this, &BKMWidget::update_backupFileList);
	connect(ui->backupItemList, &QListWidget::customContextMenuRequested, this, &BKMWidget::callMenu_backupItemList);
	connect(ui->backupFileList, &QListWidget::customContextMenuRequested, this, &BKMWidget::callMenu_backupFileList);
	this->setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);

	this->initMenuBar();

	this->refresh();
}

void BKMWidget::addNewBackupItem()
{
	AddBkDialog newDialog(this);
	if (newDialog.exec() == QDialog::Accepted) {
		bkManager.addBackup(newDialog.getUserInput());
		update_backupItemList();
		showMsg("添加存档成功");
		bkManager.saveAll();
	}
}

void BKMWidget::saveConfigs()
{
	bkManager.saveAll();
	showMsg("保存成功");
}

void BKMWidget::handeAutoSave()
{
	bool isRunning = bkManager.isAutoSaveEnabled();

	if (!isRunning) {
		bkManager.startAutoSave();
		showMsg("自动存档：启用。剩余" + QString::number(bkManager.getAutoSaveTimer()) + "秒");
	}
	else {
		auto time = bkManager.getAutoSaveTimer();
		bkManager.stopAutoSave();
		showMsg("自动存档：禁用。剩余" + QString::number(time) + "秒");
	}
	isRunning = !isRunning;
	update_autoSaveBtnStat();
}

void BKMWidget::onItemChange_bkNameList(QListWidgetItem* item)
{
	ui->backupItemList->blockSignals(true);
	bkManager.resetAutoSave();
	if (item->checkState() == Qt::Checked) {
		int checkedItemPos = -1;
		for (int i = 0; i < ui->backupItemList->count(); ++i) {
			QListWidgetItem* otherItem = ui->backupItemList->item(i);
			if (otherItem != item) {
				otherItem->setCheckState(Qt::Unchecked);
			}
			else {
				checkedItemPos = i;
			}
		}
		bkManager.setCurrentItem(checkedItemPos);
	}
	else {
		bkManager.setCurrentItem(-1);
	}
	ui->backupItemList->blockSignals(false);
}

void BKMWidget::callMenu_backupItemList(const QPoint& pos)
{
	QListWidgetItem* item = ui->backupItemList->itemAt(pos);
	int index = ui->backupItemList->row(item);
	if (item) {
		QMenu menu;
		QMenu* browseMenu = menu.addMenu(tr("浏览"));
		QAction* browseSourceAction = browseMenu->addAction(tr("源文件"));
		QAction* browseBackupAction = browseMenu->addAction(tr("存档文件夹"));

		QAction* editAction = menu.addAction(tr("编辑"));

		QMenu* deleteMenu = menu.addMenu(tr("移除"));
		QAction* removeAction = deleteMenu->addAction(tr("移除"));
		QAction* deleteAction = deleteMenu->addAction(tr("删除"));

		QMenu* moveMenu = menu.addMenu(tr("移动"));
		QAction* moveUpAction = moveMenu->addAction(tr("上移"));
		QAction* moveDownAction = moveMenu->addAction(tr("下移"));

		QAction* selectedAction = menu.exec(ui->backupItemList->viewport()->mapToGlobal(pos));
		if (selectedAction == deleteAction) {
			callBackupItemDeleteDialog(item);
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
			update_backupItemList();
		}
		else if (selectedAction == editAction) {
			AddBkDialog newDialog(this, bkManager.getBackup(index));
			int res = newDialog.exec();
			if (res == QDialog::Accepted) {
				bkManager.editBackup(newDialog.getUserInput(), index);
			}
			update_backupItemList();
		}
		else if (selectedAction == moveUpAction || selectedAction == moveDownAction) {
			int index = ui->backupItemList->currentRow();
			int next_index = -1;
			if (index == -1) {
				return;
			}
			else if (selectedAction == moveUpAction && index - 1 >= 0) {
				next_index = index - 1;
			}
			else if (selectedAction == moveDownAction && static_cast<unsigned long long>(index + 1) <= bkManager.getSize() - 1) {
				next_index = index + 1;
			}
			if (next_index != -1) {
				bkManager.swapBackups(index, next_index);
				update_backupItemList();
				ui->backupItemList->setCurrentRow(next_index);
			}
		}
	}
}

void BKMWidget::callMenu_backupFileList(const QPoint& pos)
{
	QListWidgetItem* item = ui->backupFileList->itemAt(pos);
	int index = ui->backupFileList->row(item);
	auto backupFiles = bkManager.getAllBackups();
	if (VectorTool::isValid(backupFiles, index)) {
		QMenu menu;
		QAction* copyAction = menu.addAction(tr("复制"));
		QAction* deleteAction = menu.addAction(tr("删除"));
		QAction* browseAction = menu.addAction(tr("浏览"));
		QAction* selectedAction = menu.exec(ui->backupFileList->viewport()->mapToGlobal(pos));
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
		update_backupFileList();
	}
}

void BKMWidget::refresh()
{
	static QMutex updateMtx;
	QMutexLocker locker(&updateMtx);
	update_backupFileList();
	update_backupItemList();
	update_autoSaveBtnStat();
}

void BKMWidget::openSettings()
{
	SettingsDialog newDialog(this, bkManager.getConfigs());
	int res = newDialog.exec();
	if (res == QDialog::Accepted) {
		bkManager.updateConfigs(newDialog.getUserInput());
		bkManager.resetAutoSave();
		showMsg("保存成功");
	}
}

void BKMWidget::handleQSQL(Action action)
{
	static qint64 lastClickTime = 0;
	qint64 now = QDateTime::currentMSecsSinceEpoch();
	if (now - lastClickTime < 2000) {
		QApplication::beep();
		this->showMsg("你点的太快了");
		return;
	}
	if (bkManager.getBackup(BackupManagerQt::CURRENT).empty()) {
		QApplication::beep();
		this->showMsg("未选中存档文件");
		return;
	}
	if (action == Action::QS) {
		bkManager.createBackup();
		showMsg("已添加任务");
	}
	else if (action == Action::QL) {
		bkManager.rollBack(0);
		showMsg("已添加任务");
	}
	else if (action == Action::S) {
		SaveDialog dialog(this);
		auto ret = dialog.exec();
		if (ret == QDialog::Accepted) {
			auto saveName = dialog.getUserInput();
			auto backups = bkManager.getAllBackups();
			if (std::any_of(backups.begin(), backups.end(), [saveName](auto item) {return item.first.filename() == saveName; }))
			{
				QMessageBox msg(this);
				msg.setWindowTitle("请确认");
				msg.setText(QString::fromStdWString(L"要覆盖存档 " + saveName + L"吗？"));
				msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
				msg.setDefaultButton(QMessageBox::No);
				int ret = msg.exec();
				if (ret == QMessageBox::Yes) {
					bkManager.createBackup(saveName);
				}
				showMsg("已添加任务");
			}
			else {
				bkManager.createBackup(saveName);
				showMsg("已添加任务");
			}
		}

	}
	else if (action == Action::L) {
		auto backups = bkManager.getAllBackups();
		if (backups.empty()) {
			showMsg("失败：存档列表为空");
			return;
		}
		int index = ui->backupFileList->currentRow();
		//todo 加入回档确认选项
		if (index == -1)index = 0;
		if (index >= 0 && index <= backups.size() - 1) {
			QString name = QString::fromStdWString(backups[index].first.filename().wstring());
			QMessageBox msg(this);
			//msg.setMinimumSize(200, 300);
			msg.setWindowTitle("执行回档");
			msg.setText("真的要回档\n" + name + "\n吗?");
			msg.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
			int res = msg.exec();
			if (res == QMessageBox::Ok) {
				bkManager.rollBack(index);
			}
			showMsg("已添加任务");
		}
	}
	else {
		showMsg("失败：未知操作");
		Logger::warning(__func__ + tr("Unknown action"));
	}
	lastClickTime = now;
}

void BKMWidget::initMenuBar()
{
	QVBoxLayout* menuLayout = new QVBoxLayout(this);
	QMenuBar* menuBar = new QMenuBar(this);
	menuLayout->setMenuBar(menuBar);

	auto fileMenu = menuBar->addMenu("File");
	connect(fileMenu->addAction("Add new"), &QAction::triggered, [this] {addNewBackupItem(); });
	connect(fileMenu->addAction("Delete"), &QAction::triggered, [this] {
		auto items = QWidgetTool::checkedItems(ui->backupItemList);
		if (!items.empty()) {
			this->callBackupItemDeleteDialog(items.front());
		}
		else {
			showMsg("请勾选一个存档");
		}
	});
	connect(fileMenu->addAction("Edit"), &QAction::triggered, [this] {
		auto items = QWidgetTool::checkedItems(ui->backupItemList);
		if (!items.empty()) {
			auto index = ui->backupItemList->row(items.front());
			AddBkDialog newDialog(this, bkManager.getBackup(index));
			int res = newDialog.exec();
			if (res == QDialog::Accepted) {
				bkManager.editBackup(newDialog.getUserInput(), index);
			}
			refresh();
		}
		else {
			showMsg("请勾选一个存档");
		}
	});
	auto compressMenu = fileMenu->addMenu("Compress..");
	connect(compressMenu->addAction("Compress"), &QAction::triggered, [this]() {Logger::info("Unsupported action..."); });
	connect(compressMenu->addAction("Decompress"), &QAction::triggered, [this]() {Logger::info("Unsupported action..."); });
	connect(fileMenu->addAction("Quit"), &QAction::triggered, [this] {this->close(); });

	auto editMenu = menuBar->addMenu("Backup");
	connect(editMenu->addAction("Save"), &QAction::triggered, [this] {handleQSQL(Action::S); });
	connect(editMenu->addAction("Load"), &QAction::triggered, [this] {handleQSQL(Action::L); });
	connect(editMenu->addAction("Delete"), &QAction::triggered, [this] {
		int index = ui->backupFileList->currentRow();
		auto backupFiles = bkManager.getAllBackups();
		if (VectorTool::isValid(backupFiles, index)) {
			bkManager.deleteBackup(index);
		}
		else {
			showMsg("请选中存档文件");
		}
	});
	connect(editMenu->addAction("Copy"), &QAction::triggered, [this] {
		auto index = ui->backupFileList->currentRow();
		auto backupFiles = bkManager.getAllBackups();
		if (VectorTool::isValid(backupFiles, index)) {
			bkManager.copyBackup(index);
		}
		else {
			showMsg("请选中存档文件");
		}
	});
	auto autoMenu = editMenu->addMenu("AutoSave");
	connect(autoMenu->addAction("Start"), &QAction::triggered, [this] {bkManager.startAutoSave(); refresh(); });
	connect(autoMenu->addAction("Stop"), &QAction::triggered, [this] {bkManager.stopAutoSave();  refresh(); });
	connect(autoMenu->addAction("Reset"), &QAction::triggered, [this] {bkManager.resetAutoSave(); refresh(); });

	auto viewMenu = menuBar->addMenu("View");
	connect(viewMenu->addAction("Refresh"), &QAction::triggered, [this] {
		this->refresh();
	});

	auto settingsMenu = menuBar->addMenu("Settings");
	connect(settingsMenu->addAction("Preferences"), &QAction::triggered, [this] {
		this->openSettings();
	});

	auto helpMenu = menuBar->addMenu("Help");
	connect(helpMenu->addAction("View Help"), &QAction::triggered, [] {
		QDesktopServices::openUrl(QUrl("https://github.com/Flaaax/BackupManager64#readme"));
	});
	connect(helpMenu->addAction("About"), &QAction::triggered, [this] {
		QMessageBox::about(this, tr("About"),
		tr("This is a <b>Qt application</b>.<br>"
		"Version 1.0.0<br>"
		   "Copyright © 2024 Flaaax."));
	});
}


void BKMWidget::initLogger()
{
	ui->loggerEdit->setReadOnly(true);
	ui->loggerEdit->setFont(QFont("Consolas", 8));
	connect(&Logger::instance(), &Logger::messageRequest, this, [this](const QString& msg) {
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
	connect(&Logger::instance(), &Logger::updateRequest, this, &BKMWidget::refresh);
	Logger::instance().installWidgetLogger();
	Logger::debug("loggerEdit successfully set");
}

void BKMWidget::initButtonIcon()
{
	//ui->pauseButton->setIcon(QIcon("resources//redsquare.png"));
	//ui->pauseButton->setIconSize(QSize(60, 60));
	//ui->startButton->setIcon(QIcon("resources//greenarrow.png"));
	//ui->pauseButton->setIconSize(QSize(60, 60));
	ui->myPicture->setIcon(QIcon("resources//wha.png"));
	ui->myPicture->setIconSize(QSize(121, 121));
	/*
	ui->moveBtn_down->setIcon(QIcon("resources//black_arrow2.png"));
	ui->moveBtn_down->setIconSize(QSize(25, 25));
	ui->moveBtn_up->setIcon(QIcon("resources//black_arrow1.png"));
	ui->moveBtn_up->setIconSize(QSize(25, 25));
	*/
	//ui->settingsBtn->setIcon(QIcon("resources//settings.png"));
	//ui->settingsBtn->setIconSize(QSize(26, 26));
}

void BKMWidget::update_backupItemList()
{
	ui->backupItemList->blockSignals(true);
	ui->backupItemList->clear();
	if (bkManager.getSize() > 0) {
		for (int index = 0; index < bkManager.getSize(); index++) {
			QListWidgetItem* item = new QListWidgetItem(QString::fromStdWString(bkManager.getBackup(index).name), ui->backupItemList);
			item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
			if (index == bkManager.getConfigs().currentItem) {
				item->setCheckState(Qt::Checked);
			}
			else {
				item->setCheckState(Qt::Unchecked);
			}
		}
	}
	//else {
	//	QListWidgetItem* item = new QListWidgetItem(QString("<空>"), ui->backupItemList);
	//}
	ui->backupItemList->blockSignals(false);
}

void BKMWidget::showMsg(const QString& message)
{
	QLabel* msg = new QLabel(message, this);
	msg->setGeometry(270, 40, 301, 21);
	QFont font;
	font.setFamily("微软雅黑");
	font.setPointSize(10);
	msg->setFont(font);
	msg->show();
	// 上升动画
	QPropertyAnimation* riseAnimation = new QPropertyAnimation(msg, "geometry");
	riseAnimation->setDuration(1200); // 动画持续时间
	riseAnimation->setStartValue(msg->geometry());
	riseAnimation->setEndValue(QRect(msg->x(), msg->y() - 50, msg->width(), msg->height())); // 上升效果
	// 渐变动画
	QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(msg);
	msg->setGraphicsEffect(effect);
	QPropertyAnimation* opacityAnimation = new QPropertyAnimation(effect, "opacity");
	opacityAnimation->setDuration(1000); // 持续时间
	opacityAnimation->setStartValue(0.9); // 开始透明度
	opacityAnimation->setEndValue(0.0); // 结束透明
	// 删除label
	QObject::connect(riseAnimation, &QPropertyAnimation::finished, msg, &QLabel::deleteLater);
	riseAnimation->start(QPropertyAnimation::DeleteWhenStopped);
	opacityAnimation->start(QPropertyAnimation::DeleteWhenStopped);
	//loggerEdit::append(message);
}

void BKMWidget::update_autoSaveBtnStat()
{
	bool isRunning = bkManager.isAutoSaveEnabled();
	//ui->startButton->setEnabled(!isRunning);
	//ui->pauseButton->setEnabled(isRunning);
	if (isRunning) {
		ui->autoSaveBtn->setIcon(QIcon("resources//redsquare.png"));
	}
	else {
		ui->autoSaveBtn->setIcon(QIcon("resources//greenarrow.png"));
	}
	ui->autoSaveBtn->setIconSize(ui->autoSaveBtn->size());
}

void BKMWidget::update_backupFileList()
{
	ui->backupFileList->blockSignals(true);
	ui->backupFileList->clear();
	auto backups = bkManager.getAllBackups();
	//int currentItem = bkManager.getConfigs().currentItem;
	//QString name = QString::fromStdString(bkManager.getBackup(currentItem).name);
	if (!backups.empty()) {
		for (const auto& bk : backups) {
			QListWidgetItem* item = new QListWidgetItem(ui->backupFileList);
			std::string time = ConfigHelper::timeSinceModification(bk.first);
			QWidget* listItemWidget = createAllbkListItem(bk.first);
			item->setSizeHint(listItemWidget->sizeHint());
			ui->backupFileList->setItemWidget(item, listItemWidget);
			ui->backupFileList->addItem(item);
		}
	}
	else {
		QListWidgetItem* item = new QListWidgetItem("<空>", ui->backupFileList);
	}
	ui->backupFileList->blockSignals(false);
}

bool BKMWidget::check_backupValid()
{
	if (bkManager.isBackupValid(BackupManagerQt::ALL)) {
		QMessageBox msg(this);
		msg.setWindowTitle("错误");
		QString text = "存档 " + QString::fromStdWString(bkManager.getBackup().name) + " 找不到路径，\n或者路径被修改";
		msg.setText(text);
		QPushButton* btn = msg.addButton(tr("确定"), QMessageBox::AcceptRole);
		msg.exec();
		return false;
	}
	return true;
}

void BKMWidget::log(const QString& msg)
{
	this->ui->loggerEdit->appendPlainText(msg);
}

void BKMWidget::callBackupItemDeleteDialog(QListWidgetItem* item)
{
	if (item == nullptr) {
		QMessageBox msg(this);
		msg.setWindowTitle("提示");
		msg.setText("没有选中任何存档");
		msg.exec();
		return;
	}
	QMessageBox msgBox(this);
	msgBox.setWindowTitle(tr("确定删除选中的存档吗？"));
	QString name = item->text();
	QString text = "选择\"移除\"会将\'" + name + "\'从列表中排除\n选择\"删除全部\"会删除\'" + name + "\'中的所有存档\n";
	msgBox.setText(text);
	QPushButton* btnDetach = msgBox.addButton(tr("移除"), QMessageBox::AcceptRole);
	QPushButton* btnDelete = msgBox.addButton(tr("删除全部"), QMessageBox::DestructiveRole);
	QPushButton* btnCancel = msgBox.addButton(tr("取消"), QMessageBox::RejectRole);
	msgBox.setDefaultButton(btnCancel);
	msgBox.exec();
	//auto clickedBtn = msgBox.clickedButton();
	int index = ui->backupItemList->row(item);
	if (msgBox.clickedButton() == btnDetach) {
		bkManager.removeBackupItem(index, false);
	}
	else if (msgBox.clickedButton() == btnDelete) {
		bkManager.removeBackupItem(index, true);
		QMessageBox msg;
		msg.setWindowTitle("调试消息");
		msg.setText("测试版本中，没有任何存档被删除，请自行手动删除");
		msg.exec();
	}
	bkManager.saveAll();
	update_backupItemList();
}