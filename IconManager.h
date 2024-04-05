#pragma once
#include<qicon.h>
#include"Logger.h"


class IconManager
{
public:
	static IconManager& instance()
	{
		static IconManager ins;
		return ins;
	}

	static QIcon icon(const QString& name)
	{
		const IconManager& ins = instance();
		return ins.icons.value(name, ins.icons["default"]);
	}

private:
	IconManager()
	{
		icons["greenarrow"] = QIcon(".//resources//greenarrow.png");
		icons["redsquare"] = QIcon(".//resources//redsquare.png");
		icons["wha"] = QIcon(".//resources//wha.png");
		icons["branch"] = QIcon(".//resources//branch.png");
		icons["list"] = QIcon(".//resources//list.png");
		icons["default"] = QIcon(".//resources//default.png");
	}
	IconManager(const IconManager&) = delete;
	IconManager& operator=(const IconManager&) = delete;
	QMap<QString, QIcon> icons;
};