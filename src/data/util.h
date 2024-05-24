//
//	data/util.h
//

#ifndef _UTIL_H_
#define _UTIL_H_

// Windows
#include <Windows.h>
#include <ShlObj.h>

// C/C++
#include <vector>
#include <string>

namespace utils
{
	HRESULT WriteRegistryValue(HKEY hKey, PCWSTR pszSubKey, PCWSTR pszValueName, PCWSTR pszData);
	HRESULT RegisterShellFileExtension(PCWSTR pszFileType, PCWSTR pszHandlerName, PCWSTR pszFileTypeName);
	HRESULT UnregisterShellFileExtension(PCWSTR pszFileType, PCWSTR pszHandlerName);

	bool OpenFileExplorerDialog(const std::wstring& dialogTitle, const std::wstring& initFileName, std::string& selectedFile, bool saveMode, const std::vector<COMDLG_FILTERSPEC>& vFilters = {});
}

#endif // _UTIL_H_