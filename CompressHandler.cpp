#include "CompressHandler.h"
#include "Logger.h"
#include<fstream>
#include<qprocess.h>
#include<qfileinfo.h>
#include<qdir.h>
#include<qtextcodec.h>
#include"StdStrTool.h"

namespace fs = std::filesystem;

/*
void CompressHandler::compress(const std::filesystem::path& sourcePath, const std::filesystem::path& targetDir)
{

	if (!fs::exists(sourcePath)) {
		Logger::warning("source path does not exist");
		return;
	}
	if (!fs::is_directory(targetDir)) {
		if (fs::exists(targetDir)) {
			Logger::warning("targetDir is not a directory");
			return;
		}
		fs::create_directory(targetDir);
	}

	archive* a = archive_write_new();
	archive_write_add_filter_zstd(a);
	archive_write_set_format_pax_restricted(a);
	fs::path targetName = sourcePath.filename().string().append(".tar.zst");
	archive_write_open_filename(a, (targetDir / targetName).string().c_str());

	if (fs::is_directory(sourcePath)) {
		for (const auto& entry : fs::recursive_directory_iterator(sourcePath)) {
			if (fs::is_regular_file(entry.path())) {
				fs::path relativePath = fs::relative(entry.path(), sourcePath.parent_path());
				addToArchive(a, entry.path(), relativePath);
			}
		}
	}
	else if (fs::is_regular_file(sourcePath)) {
		addToArchive(a, sourcePath,sourcePath.filename());
	}
	else {
		Logger::log("source is neither directory nor regular file");
	}

	archive_write_close(a);
	archive_write_free(a);
}

void CompressHandler::extract(const std::filesystem::path& archivePath, const std::filesystem::path& targetDir)
{
	try {
		archive* a;
		archive_entry* entry;
		int r;

		a = archive_read_new();
		archive_read_support_filter_zstd(a);
		archive_read_support_format_tar(a);

		r = archive_read_open_filename(a, archivePath.string().c_str(), 10240); // Note: Buffer size is 10240
		if (r != ARCHIVE_OK) {
			Logger::fatal("failed to open archive objects");
		}

		while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
			fs::path filePath = targetDir / archive_entry_pathname(entry);
			if (archive_entry_filetype(entry) == AE_IFDIR) {
				fs::create_directories(filePath);
			}
			else {
				fs::create_directories(filePath.parent_path());
				std::ofstream file(filePath, std::ios::binary);
				const void* buff;
				size_t size;
				la_int64_t offset;

				while (archive_read_data_block(a, &buff, &size, &offset) == ARCHIVE_OK) {
					file.write(reinterpret_cast<const char*>(buff), size);
				}
			}
		}

		r = archive_read_free(a);
		if (r != ARCHIVE_OK) {
			Logger::fatal("failed to free archive objects");
		}
	}
	catch (std::exception& e) {
		Logger::warning(e.what());
	}
}

void CompressHandler::addToArchive(archive* a, const std::filesystem::path& filePath, const std::filesystem::path& relativePath)
{
	archive_entry* entry = archive_entry_new();
	archive_entry_set_pathname(entry, reinterpret_cast<const char*>(relativePath.u8string().c_str()));
	archive_entry_set_size(entry, fs::file_size(filePath));
	archive_entry_set_filetype(entry, AE_IFREG);
	archive_entry_set_perm(entry, 0644);
	archive_write_header(a, entry);

	std::ifstream file(filePath, std::ios::binary);
	static char buffer[8192];
	while (file.read(buffer, sizeof(buffer))) {
		archive_write_data(a, buffer, file.gcount());
	}
	if (!file.eof()) {
		archive_write_data(a, buffer, file.gcount()); // 写入最后一部分数据
	}

	archive_entry_free(entry);
}

*/

void CompressHandler::compress(const std::filesystem::path& sourcePath, const std::filesystem::path& targetDir)
{
	if (!fs::exists(sourcePath)) {
		Logger::warning("source path does not exist!");
	}
	if (!fs::is_directory(targetDir)) {
		fs::create_directories(targetDir);
	}

	//Logger::debug(StrHelper::toQString(sourcePath));
	//Logger::debug(StrHelper::toQString(targetDir));


	QProcess process;
	QStringList arguments;

	auto targetName = StrHelper::toQString(sourcePath.filename());

	auto arg1 = StrHelper::toQString(targetDir) + "\\" + targetName + COMP_EXT;

	if (fs::is_directory(sourcePath)) {
		arguments << "a" << arg1 << StrHelper::toQString(sourcePath).append("\\*") << "-r";
	}
	else if (fs::is_regular_file(sourcePath)) {
		arguments << "a" << arg1 << StrHelper::toQString(sourcePath);
	}
	else {
		Logger::warning("Source is neither a directory nor a regular file");
		return;
	}

	process.start(EXE_PATH, arguments);

	if (!process.waitForFinished(-1)) {
		Logger::warning("Failed to compress: " + process.errorString());
		QByteArray standardError = process.readAllStandardError();
		QString errorString = QString::fromLocal8Bit(standardError);
		Logger::warning(errorString);
	}
	else {
		Logger::debug("Compress success!");
	}
}

void CompressHandler::extract(const std::filesystem::path& archivePath, const std::filesystem::path& targetDir)
{
	Logger::debug(StrHelper::toQString(archivePath));
	Logger::debug(StrHelper::toQString(targetDir));

	if (!std::filesystem::exists(archivePath)) {
		Logger::warning("Archive path does not exist");
		return;
	}
	if (!std::filesystem::is_directory(targetDir)) {
		std::filesystem::create_directories(targetDir);
	}

	QProcess process;
	QStringList arguments;

	QString qArchivePath = StrHelper::toQString(archivePath);
	QString qTargetDir = StrHelper::toQString(targetDir);

	arguments << "x" << qArchivePath << "-o" + qTargetDir << "-y";

	process.start(EXE_PATH, arguments);

	if (!process.waitForFinished(-1)) {
		auto error = process.readAllStandardError();
		auto output = process.readAllStandardOutput();
		Logger::debug(QString::fromLocal8Bit(output));
		Logger::warning("Failed to extract: " + QString::fromLocal8Bit(error));
		return;
	}

}

bool CompressHandler::isRegularCompFile(const std::filesystem::path& path)
{
	return fs::is_regular_file(path) && path.extension() == COMP_EXT;
}
