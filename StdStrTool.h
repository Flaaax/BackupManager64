#pragma once
#include <string>
#include <locale>
#include <codecvt>
#include <qstring.h>

class StdStrTool
{
public:
	static std::wstring toWstring(const std::string& str)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.from_bytes(str);
	}

	static std::u8string toU8string(const std::string& s)
	{
		return std::u8string(reinterpret_cast<const char8_t*>(s.data()), s.size());
	}

	static std::string toStdString(const std::wstring& s)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
		return myconv.to_bytes(s);
	}

	static std::string toStdString(const std::u8string& s)
	{
		return std::string(reinterpret_cast<const char*>(s.data()), s.size());
	}
};

