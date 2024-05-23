//
//	data/util.cpp
//

#include "util.h"

bool utils::OpenFileExplorerDialog(HWND hWnd, const wstring& dialogTitle, const wstring& initFileName, string& selectedFile, bool saveMode, const vector<COMDLG_FILTERSPEC>& vFilters /* = {}*/)
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
		hr = pfd->Show(hWnd);

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
