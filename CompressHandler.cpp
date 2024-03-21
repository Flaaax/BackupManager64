#include "CompressHandler.h"
#include "Logger.h"
#include<fstream>

namespace fs = std::filesystem;

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
                addToArchive(a, entry.path());
            }
        }
    }
    else if (fs::is_regular_file(sourcePath)) {
        addToArchive(a, sourcePath);
    }
    else {
        Logger::log("source is neither directory nor regular file");
    }

    archive_write_close(a);
    archive_write_free(a);
}

void CompressHandler::extract(const std::filesystem::path& archivePath, const std::filesystem::path& targetDir)
{
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

void CompressHandler::addToArchive(archive* a, const std::filesystem::path& filePath)
{
    archive_entry* entry = archive_entry_new();
    archive_entry_set_pathname(entry, filePath.string().c_str());
    archive_entry_set_size(entry, fs::file_size(filePath));
    archive_entry_set_filetype(entry, AE_IFREG);
    archive_entry_set_perm(entry, 0644);
    archive_write_header(a, entry);

    std::ifstream file(filePath, std::ios::binary);
    static char buffer[8192];                         //not multithread friendly
    while (file.read(buffer, sizeof(buffer))) {
        archive_write_data(a, buffer, file.gcount());
    }
    if (!file.eof()) {
        archive_write_data(a, buffer, file.gcount()); // 写入最后一部分数据
    }

    archive_entry_free(entry);
}
