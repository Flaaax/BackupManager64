#pragma once
#include<qplaintextedit.h>
#include <mutex>

//todo maybe add func like debug()
class Logger
{
public:
	enum LogType {
		DEBUG,
		INFO,
		LOG,
		FATAL,				//will call abort()
		CRITICAL,
		WARNING,
		ERR					//will throw std::runtime_error
	};

	static bool debugMode;

	static void setGlobalLogger(QPlainTextEdit* newLogger);
	static void append(const QString& msg);
	static void append(const std::wstring& msg);
	static void append(const std::string& msg);
	static void append(const char* msg);
	static void setDefaultLogger();

	static void log(const QString& msg, LogType type = LOG);

private:
	static void customMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);
	static QPlainTextEdit* logger;
	static std::mutex loggerMtx;
};

