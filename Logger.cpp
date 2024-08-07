#include "Logger.h"
#include "BackupManager/BackupConfig.h"
#include "StdStrTool.h"
#include <qtextstream.h>
#include <QMessageBox>

bool Logger::debugMode = true;

void Logger::append(const QString& msg)
{
	emit messageRequest(QString::fromStdWString(ConfigHelper::getTimeStringForLogger() + L"> ") + msg);
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

bool Logger::checkErrorCode(const QString& msg)
{
	for (int i = 0; i < msg.size(); ++i) {
		ushort code = msg[i].unicode();
		if (!((code >= 0x0020 && code <= 0x007E)
			  || (code >= 0x4E00 && code <= 0x9FFF)
			  || (code >= 0x3040 && code <= 0x309F)
			  || (code >= 0x30A0 && code <= 0x30FF)
			  || (code == 0x3000)
			  || (code >= 0xFF01 && code <= 0xFFEF))) {
			return true;
		}
	}
	return false;
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

void Logger::warning(const QString& msg)
{
	Logger::log(msg, Logger::WARNING);
}

void Logger::fatal(const QString& msg)
{
	Logger::log(msg, Logger::FATAL);
}

void Logger::critical(const QString& msg)
{
	Logger::log(msg, Logger::CRITICAL);
}

void Logger::err(const QString& msg)
{
	Logger::log(msg, Logger::ERR);
}

void Logger::err(const std::exception& e)
{
	Logger::err(e.what());
}

void Logger::debug(const QString& msg)
{
	Logger::log(msg, Logger::DEBUG);
}

void Logger::installGlobalLogger()
{
	qInstallMessageHandler(Logger::customLogger);
}

void Logger::log(const QString& msg, MsgType type)
{
	switch (type) {
		case MsgType::LOG:
			Logger::instance().append(msg);
			break;
		case MsgType::DEBUG:
			if (Logger::debugMode)Logger::instance().append("Debug: " + msg);
			break;
		case MsgType::CRITICAL:
			Logger::instance().append("Critical: " + msg);
			QMessageBox::critical(nullptr, "Critical", msg);
			break;
		case MsgType::WARNING:
			Logger::instance().append("Warning: " + msg);
			break;
		case MsgType::FATAL:
			Logger::instance().append("FATAL: " + msg);
			FileLogger::log("FATAL: " + msg);
			QMessageBox::critical(nullptr, "FATAL", msg);
			std::abort();			//program abort
			break;
		case MsgType::ERR:
			Logger::instance().append("ERROR: " + msg);
			FileLogger::log("ERROR: " + msg);
			throw std::runtime_error(msg.toStdString());
			break;
		default:
			Logger::instance().append("Unkown: " + msg);
	}
	if (Logger::debugMode && Logger::checkErrorCode(msg)) {
		Logger::instance().append("Warning: message might contain error code");
	}
}


void FileLogger::customLogger(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
	FileLogger::log(msg);
}

void FileLogger::installGlobalLogger()
{
	qInstallMessageHandler(FileLogger::customLogger);
}

void FileLogger::log(const QString& msg)
{
	QFile file(LOG_FILE);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Append))
	{
		QTextStream(stderr) << "Cannot open log file: " << file.errorString();
		return;
	}
	QTextStream out(&file);
	out << msg << "\n";
	file.flush(); // 确保消息被写入文件
}
