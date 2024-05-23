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

using namespace std;

namespace utils
{
	bool OpenFileExplorerDialog(HWND hWnd, const wstring& dialogTitle, const wstring& initFileName, string& selectedFile, bool saveMode, const vector<COMDLG_FILTERSPEC>& vFilters = {});
}

#endif // _UTIL_H_