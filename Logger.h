#pragma once
#include<qplaintextedit.h>
#include <mutex>
#include <qstring.h>
#include <qmutex.h>
#include <qtextstream.h>


class Logger :public QObject
{
	Q_OBJECT

public:
	enum MsgType {
		ERR,
		FATAL,
		LOG,
		WARNING,
		CRITICAL,
		DEBUG
	};

	static bool debugMode;

	static inline Logger& instance()
	{
		static Logger instance;
		return instance;
	}

	Logger(const Logger&) = delete;
	Logger& operator=(const Logger&) = delete;

	void append(const QString& msg);
	void append(const std::wstring& msg);
	void append(const std::string& msg);
	void append(const char* msg);
	void installGlobalLogger();
	void update(bool stat);

	static void log(const QString& msg, MsgType type = MsgType::LOG);
	static void log(const std::wstring& msg, MsgType type = MsgType::LOG) { log(QString::fromStdWString(msg), type); }
	static void log(const std::string& msg, MsgType type = MsgType::LOG) { log(QString::fromStdString(msg), type); }
	static void log(const char* msg, MsgType type = MsgType::LOG) { log(QString(msg), type); }
	static void debug(const QString& msg);
	static void warning(const QString& msg);
	static void err(const QString& msg);
	static void err(const std::exception& e);
	static void fatal(const QString& msg);
	static void critical(const QString& msg);
	
	static bool checkErrorCode(const QString& msg);		//check chinese, ascii, japanese

signals:
	void messageRequest(const QString& msg);
signals:
	void updateRequest(bool stat);

private:
	static void customLogger(QtMsgType type, const QMessageLogContext& context, const QString& msg);
	Logger() {}
};

class FileLogger {
public:
	static constexpr auto LOG_FILE = "log.txt";

	static void installGlobalLogger();
	static void log(const QString& msg);

private:
	static void customLogger(QtMsgType type, const QMessageLogContext& context, const QString& msg);
};