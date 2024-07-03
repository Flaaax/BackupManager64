#pragma once
#include<qicon.h>
#include<qfileinfo.h>
#include"Logger.h"


class IconManager {
public:
	static IconManager& instance() {
		static IconManager ins;
		return ins;
	}

	static QIcon icon(const QString& name) {
		const IconManager& ins = instance();
		return ins.icons.value(name, ins.icons["default"]);
	}

	static QString checkSuffix(const QString& filename) {
		if (QFileInfo(filename).suffix().isEmpty()) {
			return filename + ".png";
		}
		return filename;
	}

	static QString iconPath(const QString& filename) {
		return ".//resources//" + checkSuffix(filename);
	}

	void loadIcon(const QString& iconName) {
		icons[QFileInfo(iconName).completeBaseName()] = QIcon(iconPath(iconName));
	}

	void loadIcons(const QStringList& iconNames) {
		for (auto& iconName : iconNames) {
			loadIcon(iconName);
		}
	}

private:
	IconManager() {
		QStringList iconNames = { "greenarrow","redsquare","wha","branch","list","default","load","delete","rename" };
		loadIcons(iconNames);
	}
	IconManager(const IconManager&) = delete;
	IconManager& operator=(const IconManager&) = delete;

	QMap<QString, QIcon> icons;
};