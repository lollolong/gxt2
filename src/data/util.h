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
	bool OpenFileExplorerDialog(const std::wstring& dialogTitle, const std::wstring& initFileName, std::string& selectedFile, bool saveMode, const std::vector<COMDLG_FILTERSPEC>& vFilters = {});
}

#endif // _UTIL_H_