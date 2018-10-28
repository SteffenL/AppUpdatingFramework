#ifdef _WIN32
#include "FileVersionInfo_msw.h"

#include <nowide/convert.hpp>
#include <memory>
#include <Windows.h>

#pragma comment(lib, "Version.lib")

namespace aufw {

FileVersionInfo_msw::FileVersionInfo_msw(const std::string& filePath) : FileVersionInfoBase() {
    // Convert file path to UTF-16
    std::wstring filePathW = nowide::widen(filePath);
	DWORD blockSize = ::GetFileVersionInfoSizeW(filePathW.c_str(), NULL);
	if (!blockSize)
	{
		return;
	}

	std::unique_ptr<char> blockData(new char[blockSize]);
	if (!::GetFileVersionInfoW(filePathW.c_str(), 0, blockSize, &*blockData))
	{
		return;
	}

	queryFileVersion(&*blockData);
}

void FileVersionInfo_msw::queryFileVersion(char* blockData) {
	// Get fixed file info
	VS_FIXEDFILEINFO* vi;
	UINT len;
	if (!::VerQueryValueW(&*blockData, L"\\", (LPVOID*)&vi, &len))
	{
		return;
	}

	FileVersion.Major = (vi->dwFileVersionMS >> 16);
	FileVersion.Minor = (vi->dwFileVersionMS & 0xffff);
	FileVersion.Build = (vi->dwFileVersionLS >> 16);
	FileVersion.Private = (vi->dwFileVersionLS & 0xffff);

	HasFileVersion = true;
}

} // namespace
#endif
