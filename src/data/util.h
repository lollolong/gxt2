//
//	data/util.h
//

#ifndef _UTIL_H_
#define _UTIL_H_

#if _WIN32
// Windows
#include <Windows.h>
#include <ShlObj.h>
#endif

// C/C++
#include <vector>
#include <string>

namespace utils
{
	std::string ToLower(const std::string& str);
	std::string ToUpper(const std::string& str);

#if _WIN32
	HRESULT WriteRegistryValue(HKEY hKey, PCWSTR pszSubKey, PCWSTR pszValueName, PCWSTR pszData);
	HRESULT RegisterShellFileExtension(PCWSTR pszFileType, PCWSTR pszHandlerName, PCWSTR pszFileTypeName);
	HRESULT UnregisterShellFileExtension(PCWSTR pszFileType, PCWSTR pszHandlerName);
#endif

	bool OpenFileExplorerDialog(const std::string& dialogTitle, const std::string& initFileName, std::string& selectedFile, bool saveMode, const std::vector<std::string> vFilters = {});
}

#endif // !_UTIL_H_