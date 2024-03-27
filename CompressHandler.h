#pragma once
#include <filesystem>


class CompressHandler
{
public:
    static constexpr auto EXE_PATH = "./bin/7z.exe";

    static void compress(const std::filesystem::path& sourcePath, const std::filesystem::path& targetDir);
    static void extract(const std::filesystem::path& archivePath, const std::filesystem::path& targetDir);

private:
};

