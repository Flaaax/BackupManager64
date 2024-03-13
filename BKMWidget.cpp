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
#include <qdebug.h>
#include "Logger.h"

static void myFileLogger(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
	QFile file("log.txt");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Append)) {
		// 如果文件无法打开，则输出到标准错误输出
		QTextStream(stderr) << "Cannot open log file: " << file.errorString();
		return;
	}

	QTextStream out(&file);
	out << msg << "\n"; // 将消息追加到文件中
	file.flush();  // 确保消息立即写入文件

	// 如果需要，也可以将消息输出到标准输出
	QTextStream(stderr) << msg << "\n";
}

namespace fs = std::filesystem;

static QWidget* createAllbkListItem(const std::filesystem::path& filepath)
{
	QWidget* widget = new QWidget();
	QHBoxLayout* layout = new QHBoxLayout(widget);
	static const QFont font("微软雅黑");
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
	this->bkManager.setGeneralCallback([this](bool stat) {this->update_allBkList(); });
	this->initLogger();
	qInstallMessageHandler(myFileLogger);
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
	//timer->start(30000);	//每5min更新一次存档列表
	//QMessageBox::critical(nullptr, "Exception Caught", "BKMWidget init");

}

void BKMWidget::onClick_addBkButton()
{
	AddBkDialog newDialog(this);
	if (newDialog.exec() == QDialog::Accepted) {
		bkManager.addBackup(newDialog.getUserInput());
		update_backupNameList();
		showMessage("添加存档成功");
	}
}

void BKMWidget::onClick_saveButton()
{
	bkManager.saveAll();
	showMessage("保存成功");
}

void BKMWidget::onClick_SPButton()
{
	QObject* obj = sender();
	time_t bktime = bkManager.getAutoBackupTimer();
	if (obj == ui->startButton) {
		bkManager.resumeAutoBackup();
		showMessage(QString("自动存档：启用 time=") + QString::number(bktime));
	}
	else if (obj == ui->pauseButton) {
		bkManager.pauseAutoBackup();
		showMessage(QString("自动存档：禁用 time=") + QString::number(bktime));
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
		QMenu* browseMenu = menu.addMenu(tr("浏览路径"));
		QAction* browseSourceAction = browseMenu->addAction(tr("源文件路径"));
		QAction* browseBackupAction = browseMenu->addAction(tr("存档路径"));

		QAction* editAction = menu.addAction(tr("编辑"));

		QMenu* deleteMenu = menu.addMenu(tr("移除"));
		QAction* removeAction = deleteMenu->addAction(tr("移除"));
		QAction* deleteAction = deleteMenu->addAction(tr("删除"));

		QMenu* moveMenu = menu.addMenu(tr("移动"));
		QAction* moveUpAction = moveMenu->addAction(tr("上移"));
		QAction* moveDownAction = moveMenu->addAction(tr("下移"));

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
	if (index >= 0 && index <= bkManager.getSize() - 1) {
		QMenu menu;
		QAction* copyAction = menu.addAction(tr("复制"));
		QAction* deleteAction = menu.addAction(tr("删除"));
		QAction* browseAction = menu.addAction(tr("查看路径"));
		QAction* selectedAction = menu.exec(ui->allBkList->viewport()->mapToGlobal(pos));
		if (selectedAction == copyAction) {
			bkManager.copyBackup(index);
		}
		else if (selectedAction == deleteAction) {
			bkManager.deleteBackup(index);
		}
		else if (selectedAction == browseAction) {
			const auto& path = bkManager.getAllBackups()[index].first;
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

void BKMWidget::updateAll()
{
	//this->updateMtx.lock();
	update_allBkList();
	update_backupNameList();
	update_SPButtonStat();
	//this->updateMtx.unlock();
}

void BKMWidget::onClick_settingsButton()
{
	SettingsDialog newDialog(this, bkManager.getConfigs());
	int res = newDialog.exec();
	if (res == QDialog::Accepted) {
		bkManager.updateConfigs(newDialog.getUserInput());
		showMessage("保存成功");
	}
}

void BKMWidget::onClick_bk_roll_Button()
{
	qint64 now = QDateTime::currentMSecsSinceEpoch();
	if (now - lastClickTime < 2000) {
		// 如果距离上次点击不足2秒，直接返回
		/*
		QMessageBox msg(this);
		msg.setWindowTitle(":(");
		msg.setText("你点的太快了");
		msg.exec();
		*/
		QApplication::beep();
		this->showMessage("你点的太快了");
		return;
	}
	QObject* obj = sender();
	if (bkManager.getBackup(BackupManagerQt::CURRENT).empty()) {
		QApplication::beep();
		this->showMessage("错误：内容为空");
		return;
	}
	if (obj == ui->quickBkButton) {
		bkManager.createBackup(L"", false);
		lastClickTime = now;
	}
	else if (obj == ui->quickRollbackBtn) {
		showMessage("快速回档中");
		//todo 加入回档确认选项
		bkManager.rollBack(0);
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
				msg.setWindowTitle("请确认");
				msg.setText(QString::fromStdWString(L"要覆盖存档 " + saveName + L"吗？"));
				msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
				msg.setDefaultButton(QMessageBox::No);
				int ret = msg.exec();
				if (ret == QMessageBox::Yes) {
					bkManager.createBackup(saveName, true);
				}
				else {
					showMessage("已取消存档");
				}
				return;
			}
			else {
				bkManager.createBackup(saveName);
				if (bkManager.goodStatus()) {
					showMessage("存档成功");
				}
				else {
					showMessage("存档失败！");
				}
				update_allBkList();
			}
		}

	}
	else if (obj == ui->loadBtn) {
		auto backups = bkManager.getAllBackups();
		if (backups.empty()) {
			showMessage("失败：没有存档");
			return;
		}
		int index = ui->allBkList->currentRow();
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
				lastClickTime = now;
			}
		}
	}
	else {
		showMessage("失败：未知操作");
	}
}

void BKMWidget::initLogger()
{
	ui->Logger->setReadOnly(true);
	ui->Logger->setFont(QFont("Consolas", 8));
	Logger::setGlobalLogger(ui->Logger);
	Logger::setDefaultLogger();
	qDebug() << "Logger successfully set";

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
	//bkConfigs.currentItem = ui->bkNameList->currentRow();
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
	font.setFamily("微软雅黑");
	font.setPointSize(10);
	msg->setFont(font);
	msg->show();
	// up动画
	QPropertyAnimation* riseAnimation = new QPropertyAnimation(msg, "geometry");
	riseAnimation->setDuration(1200); // 动画持续时间
	riseAnimation->setStartValue(msg->geometry());
	riseAnimation->setEndValue(QRect(msg->x(), msg->y() - 50, msg->width(), msg->height())); // 上升效果
	// 设置透明度动画
	QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(msg);
	msg->setGraphicsEffect(effect);
	QPropertyAnimation* opacityAnimation = new QPropertyAnimation(effect, "opacity");
	opacityAnimation->setDuration(1000); // 持续时间
	opacityAnimation->setStartValue(0.9); // 开始透明度
	opacityAnimation->setEndValue(0.0); // 结束透明
	// 当动画完成后删除label
	QObject::connect(riseAnimation, &QPropertyAnimation::finished, msg, &QLabel::deleteLater);
	riseAnimation->start(QPropertyAnimation::DeleteWhenStopped);
	opacityAnimation->start(QPropertyAnimation::DeleteWhenStopped);
	//QTimer::singleShot(1200, msg, &QLabel::deleteLater);
	//Logger::append(message);
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
			std::string time = BackupConfig::timeSinceModification(bk.first);
			QWidget* listItemWidget = createAllbkListItem(bk.first);
			item->setSizeHint(listItemWidget->sizeHint());
			ui->allBkList->setItemWidget(item, listItemWidget);
			ui->allBkList->addItem(item);
		}
	}
	else {
		QListWidgetItem* item = new QListWidgetItem("空列表", ui->allBkList);
	}
	ui->allBkList->blockSignals(false);
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

void BKMWidget::nameListDelete_dialog(QListWidgetItem* item)
{
	QMessageBox msgBox(this);
	msgBox.setWindowTitle(tr("删除存档"));
	QString name = item->text();
	QString text = "选择\"移除\"会将 " + name + " 从列表中排除\n选择\"删除全部\"会删除 " + name + " 中的所有存档\n";
	msgBox.setText(text);
	QPushButton* btnDetach = msgBox.addButton(tr("移除"), QMessageBox::AcceptRole);
	QPushButton* btnDelete = msgBox.addButton(tr("删除全部"), QMessageBox::DestructiveRole);
	QPushButton* btnCancel = msgBox.addButton(tr("取消"), QMessageBox::RejectRole);
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
		msg.setWindowTitle("好消息");
		msg.setText("为了防止手滑，没有任何存档被实际删除:)");
		msg.exec();
	}
	bkManager.saveAll();
	update_backupNameList();
}