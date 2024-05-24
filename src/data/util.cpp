//
//	data/util.cpp
//

// Project
#include "util.h"
#include "grc/graphics.h"

// C/C++
#include <strsafe.h>

namespace utils
{
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

	bool OpenFileExplorerDialog(const std::wstring& dialogTitle, const std::wstring& initFileName, std::string& selectedFile, bool saveMode, const std::vector<COMDLG_FILTERSPEC>& vFilters /* = {}*/)
	{
		bool bSuccess = false;
		IFileDialog* pfd = NULL;

		HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		if (FAILED(hr))
		{
			return false;
		}

		if (saveMode)
		{
			hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
		}
		else
		{
			hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
		}

		if (SUCCEEDED(hr))
		{
			FILEOPENDIALOGOPTIONS fos;
			hr = pfd->GetOptions(&fos);

			if (SUCCEEDED(hr))
			{
				fos |= FOS_PATHMUSTEXIST | FOS_FILEMUSTEXIST | FOS_NOCHANGEDIR | FOS_DONTADDTORECENT;

				if (saveMode)
				{
					fos |= FOS_NOTESTFILECREATE | FOS_OVERWRITEPROMPT;
				}
				hr = pfd->SetOptions(fos);
			}

			if (!vFilters.empty())
			{
				hr = pfd->SetFileTypes((UINT)vFilters.size(), vFilters.data());
				hr = pfd->SetFileTypeIndex(1);
			}

			if (!initFileName.empty())
			{
				hr = pfd->SetFileName(initFileName.c_str());
			}
			hr = pfd->SetTitle(dialogTitle.c_str());
			hr = pfd->Show((HWND)CGraphics::GetInstance().GetWin32Window());

			if (SUCCEEDED(hr))
			{
				IShellItem* pShellItem;

				hr = pfd->GetResult(&pShellItem);
				if (SUCCEEDED(hr))
				{
					PWSTR pszBuffer = NULL;
					hr = pShellItem->GetDisplayName(SIGDN_FILESYSPATH, &pszBuffer);
					if (SUCCEEDED(hr))
					{
						const int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, pszBuffer, lstrlenW(pszBuffer), NULL, 0, NULL, NULL);
						selectedFile.clear();
						selectedFile.resize(sizeNeeded);
						WideCharToMultiByte(CP_UTF8, 0, pszBuffer, lstrlenW(pszBuffer), selectedFile.data(), sizeNeeded, NULL, NULL);
						bSuccess = true;
						CoTaskMemFree(pszBuffer);
					}
					pShellItem->Release();
				}
			}
			pfd->Release();
		}
		CoUninitialize();

		return bSuccess;
	}
}