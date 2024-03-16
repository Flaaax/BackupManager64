#pragma once
#include<qlistwidget.h>
class QWidgetTool
{
public:
	static QList<QListWidgetItem*> checkedItems(QListWidget* listWidget)
	{
		QList<QListWidgetItem*> items;
		if (listWidget != nullptr) {
			for (int i = 0; i < listWidget->count(); ++i) {
				QListWidgetItem* item = listWidget->item(i);
				if (item->checkState() == Qt::Checked) {
					items.push_back(item);
				}
			}
		}
		return items;
	}
};

