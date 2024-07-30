//
//	data/util.cpp
//

// Project
#include "util.h"
#include "grc/graphics.h"
#include <filesystem>

#if _WIN32
// Windows
#include <strsafe.h>
#endif

// C/C++
#include <ctype.h>

// vendor
#include <portable-file-dialogs.h>

namespace utils
{
	std::string ToLower(const std::string& str)
	{
		std::string r = str;
		std::transform(r.begin(), r.end(), r.begin(), [](const char& c) -> char
		{
			return static_cast<char>(tolower(c));
		});
		return r;
	}
	std::string ToUpper(const std::string& str)
	{
		std::string r = str;
		std::transform(r.begin(), r.end(), r.begin(), [](const char& c) -> char
		{
			return static_cast<char>(toupper(c));
		});
		return r;
	}

	// Credits Notepad++ src
	// https://github.com/notepad-plus-plus/notepad-plus-plus/blob/master/PowerEditor/src/MISC/Common/Sorters.h#L156
	int SortStringIntegers(const std::string& a, const std::string& b)
	{
		int compareResult = 0;
		size_t aNumIndex = 0;
		size_t bNumIndex = 0;
		while (compareResult == 0)
		{
			if (aNumIndex >= a.length() || bNumIndex >= b.length())
			{
				compareResult = a.compare(std::min<size_t>(aNumIndex, a.length()), std::string::npos, b, std::min<size_t>(bNumIndex, b.length()), std::string::npos);
				break;
			}

			bool aChunkIsNum = a[aNumIndex] >= '0' && a[aNumIndex] <= '9';
			bool bChunkIsNum = b[bNumIndex] >= '0' && b[bNumIndex] <= '9';

			int aNumSign = 1;
			// Could be start of negative number
			if (!aChunkIsNum && (aNumIndex + 1) < a.length())
			{
				aChunkIsNum = (a[aNumIndex] == '-' && (a[aNumIndex + 1] >= '0' && a[aNumIndex + 1] <= '9'));
				aNumSign = -1;
			}

			int bNumSign = 1;
			if (!bChunkIsNum && (bNumIndex + 1) < b.length())
			{
				bChunkIsNum = (b[bNumIndex] == '-' && (b[bNumIndex + 1] >= '0' && b[bNumIndex + 1] <= '9'));
				bNumSign = -1;
			}

			// One is number and one is string
			if (aChunkIsNum != bChunkIsNum)
			{
				compareResult = a[aNumIndex] - b[bNumIndex];

				// compareResult isn't necessarily 0
				// consider this case: "0-0", "0-"
				// "-0" is considered a number, but "-" isn't
				// but we are comparing two "-', which is the same
				aNumIndex++;
				bNumIndex++;
			}
			// Both are numbers
			else if (aChunkIsNum)
			{
				// if the sign is differemt, just return the compareResult
				if (aNumSign != bNumSign)
				{
					if (aNumSign == 1)
					{
						compareResult = 1;
					}
					else
					{
						compareResult = -1;
					}
					// No need to update anything; compareResult != 0; will break out while loop
				}
				else
				{
					if (aNumSign == -1)
					{
						aNumIndex++;
						bNumIndex++;
					}

					size_t aNumEnd = a.find_first_not_of("1234567890", aNumIndex);
					if (aNumEnd == std::string::npos)
					{
						aNumEnd = a.length();
					}

					size_t bNumEnd = b.find_first_not_of("1234567890", bNumIndex);
					if (bNumEnd == std::string::npos)
					{
						bNumEnd = b.length();
					}

					int aZeroNum = 0;
					while (aNumIndex < a.length() && a[aNumIndex] == '0')
					{
						aZeroNum++;
						aNumIndex++;
					}

					int bZeroNum = 0;
					while (bNumIndex < b.length() && b[bNumIndex] == '0')
					{
						bZeroNum++;
						bNumIndex++;
					}

					size_t aNumLength = aNumEnd - aNumIndex;
					size_t bNumLength = bNumEnd - bNumIndex;

					// aNum is longer than bNum, must be larger (smaller if negative)
					if (aNumLength > bNumLength)
					{
						compareResult = 1 * aNumSign;
						// No need to update anything; compareResult != 0; will break out while loop
					}
					// bNum is longer than aNum, must be larger (smaller if negative)
					else if (aNumLength < bNumLength)
					{
						compareResult = -1 * aNumSign;
						// No need to update anything; compareResult != 0; will break out while loop
					}
					else
					{
						// the lengths of the numbers are equal
						// compare the two number. However, we can not use std::stoll
						// because the number strings can be every large, well over the maximum long long value
						// thus, we compare each digit one by one
						while (compareResult == 0
							&& aNumIndex < a.length()
							&& (a[aNumIndex] >= '0' && a[aNumIndex] <= '9')
							&& bNumIndex < b.length()
							&& (b[bNumIndex] >= '0' && b[bNumIndex] <= '9'))
						{
							compareResult = (a[aNumIndex] - b[bNumIndex]) * aNumSign;
							aNumIndex++;
							bNumIndex++;
						}

						if (compareResult == 0)
						{
							compareResult = bZeroNum - aZeroNum;
						}
					}
				}
			}
			// Both are strings
			else
			{
				if (a[aNumIndex] == '-')
				{
					aNumIndex++;
				}
				if (b[bNumIndex] == '-')
				{
					bNumIndex++;
				}

				size_t aChunkEnd = a.find_first_of("1234567890-", aNumIndex);
				size_t bChunkEnd = b.find_first_of("1234567890-", bNumIndex);
				compareResult = a.compare(aNumIndex, aChunkEnd - aNumIndex, b, bNumIndex, bChunkEnd - bNumIndex);
				aNumIndex = aChunkEnd;
				bNumIndex = bChunkEnd;
			}
		}
		return compareResult;
	}

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

	bool OpenFileExplorerDialog(const std::string& dialogTitle, const std::string& initFileName, std::string& selectedFile, bool saveMode, const std::vector<std::string> vFilters)
	{
		if (saveMode)
		{
			pfd::save_file result = pfd::save_file(dialogTitle, (std::filesystem::current_path() / initFileName).string(), vFilters);
			selectedFile = result.result();

			return !selectedFile.empty();
		}
		else
		{
			pfd::open_file result = pfd::open_file(dialogTitle, (std::filesystem::current_path() / initFileName).string(), vFilters);

			if (result.result().size() >= 1)
			{
				selectedFile = result.result().at(0);
				return !selectedFile.empty();
			}
			return false;
		}
	}
}