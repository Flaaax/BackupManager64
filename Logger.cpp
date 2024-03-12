#include "Logger.h"
#include "BackupManager/BackupConfig.h"
#include "StdStrTool.h"

QPlainTextEdit* Logger::logger = NULL;
std::mutex Logger::loggerMtx;

void Logger::setGlobalLogger(QPlainTextEdit* newLogger)
{
	std::lock_guard<std::mutex> lock(loggerMtx);
	Logger::logger = newLogger;
}

void Logger::append(const QString& msg)
{
	std::lock_guard<std::mutex> lock(loggerMtx);
	if (logger != NULL) {
		logger->appendPlainText(QString::fromStdWString(BackupConfig::getCurTimeForLogger() + L"> ") + msg);
	}
	else {
		throw std::runtime_error("Error: logger does not exist!\n");
	}
}

void Logger::append(const std::wstring& msg)
{
	Logger::append(QString::fromStdWString(msg));
}

void Logger::append(const std::string& msg)
{
	Logger::append(QString::fromStdString(msg));
}

void Logger::append(const char* msg)
{
	Logger::append(QString::fromUtf8(msg));
}

void Logger::customMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
	QByteArray localMsg = msg.toLocal8Bit();
	switch (type) {
		case QtDebugMsg:
			Logger::append(QString("Debug: %1").arg(msg));
			break;
		case QtInfoMsg:
			Logger::append(QString("Info: %1").arg(msg));
			break;
		case QtWarningMsg:
			Logger::append(QString("Warning: %1").arg(msg));
			break;
		case QtCriticalMsg:
			Logger::append(QString("Critical: %1").arg(msg));
			break;
		case QtFatalMsg:
			Logger::append(QString("Fatal: %1").arg(msg));
			abort();
	}
}

void Logger::setDefaultLogger()
{
	qInstallMessageHandler(Logger::customMessageHandler);
}

void Logger::log(const QString& msg, LogType type)
{
	switch (type) {
		case LogType::LOG:
			Logger::append(msg);
			break;
		case LogType::DEBUG:
			Logger::append("Debug: " + msg);
			break;
		case LogType::CRITICAL:
			Logger::append("Critical: " + msg);
			break;
		case LogType::INFO:
			Logger::append("Info: " + msg);
			break;
		case LogType::WARNING:
			Logger::append("Warning: " + msg);
			break;
		case LogType::FATAL:
			Logger::append("FATAL: " + msg);
			std::abort();			//program abort
			break;
		case LogType::ERR:
			Logger::append("ERROR: " + msg);
			throw std::runtime_error(msg.toStdString());
			break;
		default:
			Logger::append("Unkown: " + msg);
	}
}
