#pragma once
#include <qwidget.h>
#include "BackupManager/BackupConfig.h"
#include <qboxlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qvector.h>
#include <functional>
#include "IconManager.h"

class BackupFileListItemWidget :public QWidget {
	Q_OBJECT
private:
	BackupFile file;
	QVector<QPushButton*> buttons;

	void resizeEvent(QResizeEvent* event) override {
		QWidget::resizeEvent(event);
		resizeButton();
	}

	void resizeButton() {
		int buttonWidth = this->height();
		for (int i = 0; i < buttons.size(); i++) {
			auto button = buttons[i];
			button->setFixedSize(buttonWidth, buttonWidth);
			button->move(button->x(), 0);
			button->setIconSize(button->size());
		}
	}

	void setButtonsIcons() {
		auto& icons = IconManager::instance();
		buttons[0]->setIcon(icons.icon("load"));
		buttons[1]->setIcon(icons.icon("rename"));
		buttons[2]->setIcon(icons.icon("delete"));
		buttons[0]->setToolTip("���ش˴浵");
		buttons[1]->setToolTip("������");
		buttons[2]->setToolTip("ɾ��");
	}

	void setButtonsCallbacks() {
		for (int i = 0; i < buttons.size(); i++) {
			auto button = buttons[i];
			connect(button, &QPushButton::clicked, this, [this, i] {emit buttonClicked(i); });
		}
	}

signals:
	void buttonClicked(int buttonId);

public:
	BackupFileListItemWidget(QWidget* parent = nullptr, BackupFile _file = BackupFile()) :QWidget(parent), file(_file) {
		QHBoxLayout* layout = new QHBoxLayout(this);
		static const QFont font("΢���ź�");
		QString backupNameText = QString::fromStdWString(file.name);
		QString timeText = QString::fromStdString(ConfigHelper::timeSinceModification(file.path) + " ");
		QLabel* leftLabel = new QLabel(backupNameText);
		QLabel* midLabel = new QLabel(timeText);
		leftLabel->setFont(font);
		midLabel->setFont(font);
		midLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

		QHBoxLayout* rightLayout = new QHBoxLayout();

		for (int i = 0; i < 3; i++) {
			auto newButton = new QPushButton();
			buttons.push_back(newButton);
			rightLayout->addWidget(newButton);
		}

		rightLayout->setSpacing(0);
		rightLayout->setContentsMargins(0, 0, 0, 0);
		layout->addWidget(leftLabel);
		layout->addWidget(midLabel);
		layout->addLayout(rightLayout);
		layout->setSpacing(0);
		// ȷ���Ҳ�ı�ǩ���������������ÿռ�
		layout->setStretch(1, 1);
		layout->setContentsMargins(3, 3, 3, 3); // ���ý�С�ı߾�
		this->setLayout(layout);
		setButtonsVisible(false);
		setButtonsIcons();
		setButtonsCallbacks();
	}

	void setButtonsVisible(bool visible) {
		for (auto button : buttons) {
			button->setVisible(visible);
		}
		resizeButton();
	}
};

