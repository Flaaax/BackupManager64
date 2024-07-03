#pragma once
#include <qlistwidget.h>
#include "BackupFileListItem.h"

class BackupFileList :public QListWidget {
	Q_OBJECT
public:
	BackupFileList(QWidget* parent = nullptr) : QListWidget(parent) {
		connect(this, &QListWidget::itemSelectionChanged, this, &BackupFileList::updateButtonVisibility);
	}

private slots:
	void updateButtonVisibility() const {
		for (int i = 0; i < count(); ++i) {
			QListWidgetItem* item = this->item(i);
			QWidget* widget = itemWidget(item);
			if (BackupFileListItemWidget* listItem = qobject_cast<BackupFileListItemWidget*>(widget)) {
				listItem->setButtonsVisible(item->isSelected());
			}
		}
	}
};

