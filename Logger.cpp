#include "Logger.h"
#include "BackupManager/BackupConfig.h"
#include "StdStrTool.h"
#include <qtextstream.h>
#include <QMessageBox>

bool Logger::debugMode = true;

void Logger::append(const QString& msg)
{
	emit messageRequest(QString::fromStdWString(ConfigHelper::getCurTimeForLogger() + L"> ") + msg);
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

void Logger::update(bool stat)
{
	emit updateRequest(stat);
}

void Logger::customLogger(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
	QByteArray localMsg = msg.toLocal8Bit();
	switch (type) {
		case QtDebugMsg:
			if (Logger::debugMode)instance().append(QString("Debug: %1").arg(msg));
			break;
		case QtInfoMsg:
			instance().append(QString("Info: %1").arg(msg));
			break;
		case QtWarningMsg:
			instance().append(QString("Warning: %1").arg(msg));
			break;
		case QtCriticalMsg:
			instance().append(QString("Critical: %1").arg(msg));
			break;
		case QtFatalMsg:
			instance().append(QString("Fatal: %1").arg(msg));
			abort();
	}
}

void Logger::fileLogger(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
	QFile file("log.txt");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Append)) {
		QTextStream(stderr) << "Cannot open log file: " << file.errorString();
		return;
	}
	QTextStream out(&file);
	out << msg << "\n";
	file.flush();
	QTextStream(stderr) << msg << "\n";
}

void Logger::warning(const QString& msg)
{
	Logger::log(msg, Logger::WARNING);
}

void Logger::fatal(const QString& msg)
{
	Logger::log(msg, Logger::FATAL);
}

void Logger::info(const QString& msg)
{
	Logger::log(msg, Logger::INFO);
}

void Logger::critical(const QString& msg)
{
	Logger::log(msg, Logger::CRITICAL);
}

void Logger::err(const QString& msg)
{
	Logger::log(msg, Logger::ERR);
}

void Logger::debug(const QString& msg)
{
	Logger::log(msg, Logger::DEBUG);
}

void Logger::installWidgetLogger()
{
	qInstallMessageHandler(Logger::customLogger);
}

void Logger::log(const QString& msg, LogType type)
{
	switch (type) {
		case LogType::LOG:
			Logger::instance().append(msg);
			break;
		case LogType::DEBUG:
			if (Logger::debugMode)Logger::instance().append("Debug: " + msg);
			break;
		case LogType::CRITICAL:
			Logger::instance().append("Critical: " + msg);
			break;
		case LogType::INFO:
			Logger::instance().append("Info: " + msg);
			break;
		case LogType::WARNING:
			Logger::instance().append("Warning: " + msg);
			break;
		case LogType::FATAL:
			Logger::instance().append("FATAL: " + msg);
			std::abort();			//program abort
			break;
		case LogType::ERR:
			Logger::instance().append("ERROR: " + msg);
			throw std::runtime_error(msg.toStdString());
			break;
		default:
			Logger::instance().append("Unkown: " + msg);
	}
}
