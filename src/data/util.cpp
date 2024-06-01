//
//	data/util.cpp
//

// Project
#include "util.h"
#include "grc/graphics.h"
#include "portable-file-dialogs.h"
#include <filesystem>

#if _WIN32

// C/C++
#include <strsafe.h>

#endif

namespace utils
{
#if _WIN32
	HRESULT WriteRegistryValue(HKEY hKey, PCWSTR pszSubKey, PCWSTR pszValueName, PCWSTR pszData)
	{
		HRESULT hr;
		HKEY hSubKey = NULL;

		hr = HRESULT_FROM_WIN32(RegCreateKeyEx(hKey, pszSubKey, 0,
			NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hSubKey, NULL));

		if (SUCCEEDED(hr))
		{
			if (pszData != NULL)
			{
				DWORD cbData = lstrlen(pszData) * sizeof(*pszData);
				hr = HRESULT_FROM_WIN32(RegSetValueEx(hSubKey, pszValueName, 0,
					REG_SZ, reinterpret_cast<const BYTE*>(pszData), cbData));
			}

			RegCloseKey(hSubKey);
		}
		return hr;
	}

	HRESULT RegisterShellFileExtension(PCWSTR pszFileType, PCWSTR pszHandlerName, PCWSTR pszFileTypeName)
	{
		HRESULT hr = S_OK;

		hr = WriteRegistryValue(HKEY_CLASSES_ROOT, pszFileType, NULL, pszHandlerName);
		if (SUCCEEDED(hr))
		{
			hr = WriteRegistryValue(HKEY_CLASSES_ROOT, pszHandlerName, NULL, pszFileTypeName);
			if (SUCCEEDED(hr))
			{
				wchar_t szSubkeyIcon[MAX_PATH] = {};
				hr = StringCchPrintf(szSubkeyIcon, ARRAYSIZE(szSubkeyIcon), TEXT("%s\\DefaultIcon"), pszHandlerName);
				if (SUCCEEDED(hr))
				{
					wchar_t szModule[MAX_PATH] = {};
					GetModuleFileName(NULL, szModule, ARRAYSIZE(szModule));

					wchar_t szDefaultIcon[MAX_PATH] = {};
					hr = StringCchPrintf(szDefaultIcon, ARRAYSIZE(szDefaultIcon), TEXT("%s,1"), szModule);
					if (SUCCEEDED(hr))
					{
						hr = WriteRegistryValue(HKEY_CLASSES_ROOT, szSubkeyIcon, NULL, szDefaultIcon);
						if (SUCCEEDED(hr))
						{
							wchar_t szSubkeyCmd[MAX_PATH] = {};
							hr = StringCchPrintf(szSubkeyCmd, ARRAYSIZE(szSubkeyCmd), TEXT("%s\\shell\\open\\command"), pszHandlerName);
							if (SUCCEEDED(hr))
							{
								wchar_t szCommand[MAX_PATH] = {};
								hr = StringCchPrintf(szCommand, ARRAYSIZE(szCommand), TEXT("\"%s\" \"%s\""), szModule, TEXT("%1"));
								if (SUCCEEDED(hr))
								{
									hr = WriteRegistryValue(HKEY_CLASSES_ROOT, szSubkeyCmd, NULL, szCommand);
								}
							}
						}
					}
				}
			}
		}
		return hr;
	}

	HRESULT UnregisterShellFileExtension(PCWSTR pszFileType, PCWSTR pszHandlerName)
	{
		RegDeleteTree(HKEY_CLASSES_ROOT, pszFileType);
		return HRESULT_FROM_WIN32(RegDeleteTree(HKEY_CLASSES_ROOT, pszHandlerName));
	}

#endif

	bool OpenFileExplorerDialog(const std::string& dialogTitle, const std::wstring& initFileName, std::string& selectedFile, bool saveMode, const std::vector<std::string> vFilters)
	{
		if (saveMode)
		{
			auto result = pfd::save_file(dialogTitle, (std::filesystem::current_path() / initFileName).string(), vFilters);
			selectedFile = result.result();

			return selectedFile.empty() == false;
		}
		else
		{
			auto result = pfd::open_file(dialogTitle, (std::filesystem::current_path() / initFileName).string(), vFilters);

			if (result.result().size() >= 1)
			{
				selectedFile = result.result().at(0);
				return selectedFile.empty() == false;
			}

			return false;
		}
	}
}