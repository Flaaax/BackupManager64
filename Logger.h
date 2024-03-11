#pragma once
#include<qplaintextedit.h>
#include <mutex>

class Logger
{
public:
	static void setGlobalLogger(QPlainTextEdit* newLogger);
	static void append(const QString& msg);
	static void append(const std::wstring& msg);
	static void append(const std::string& msg);
	static void append(const char* msg);
	static void setDefaultLogger();

private:
	static void customMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);
	static QPlainTextEdit* logger;
	static std::mutex loggerMtx;
};

