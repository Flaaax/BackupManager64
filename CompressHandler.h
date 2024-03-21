#pragma once
#include<archive.h>
#include<archive_entry.h>
#include <filesystem>


class CompressHandler
{
public:

    static void compress(const std::filesystem::path& sourcePath, const std::filesystem::path& targetDir);
    static void extract(const std::filesystem::path& archivePath, const std::filesystem::path& targetDir);

private:
    static void addToArchive(struct archive* a, const std::filesystem::path& filePath);
};

