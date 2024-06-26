#pragma once
#include <filesystem>


class CompressHandler
{
public:
	static constexpr auto EXE_PATH = "./lib/7z.exe";
	static constexpr auto COMP_EXT = ".7z";

	static void compress(const std::filesystem::path& sourcePath, const std::filesystem::path& targetDir);
	static void extract(const std::filesystem::path& archivePath, const std::filesystem::path& targetDir);
	static bool isRegularCompFile(const std::filesystem::path& path);

private:
};