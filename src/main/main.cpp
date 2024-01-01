//
//	main/main.cpp
//

// Project
#include "main.h"
#include "gxt/gxt2.h"

// C/C++
#include <fstream>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[])
{
	if (argc != 3) {
		printf("Usage: %s [/read text.gxt2] | [/create text.txt]\n", argv[0]);
		return 1;
	}

	const char* szMode			= argv[1];
	const bool bReadMode		= _stricmp(szMode, "/read") == 0;
	const bool bCreateMode		= _stricmp(szMode, "/create") == 0;

	if (!bReadMode && !bCreateMode)
	{
		printf("Unknown option: %s\n", szMode);
		return 1;
	}

	const char* szFilePath			= argv[2];
	const char* szFileName			= strrchr(szFilePath, '\\') ? strrchr(szFilePath, '\\') + 1 : szFilePath;
	const char* szFileExtension		= strrchr(szFileName, '.');

	if (!szFileExtension) {
		printf("No file extension.\n");
		return 1;
	}

	const bool bAllowedType =		!_stricmp(szFileExtension, ".gxt2") ||
									!_stricmp(szFileExtension, ".txt");

	if (!bAllowedType) {
		printf("Unknown input file format.\n");
		return 1;
	}

	ifstream ifs(szFilePath);

	if (!ifs.is_open()) {
		printf("The specified file could not be opened.\n");
		return 1;
	}

	if (bReadMode)
	{
		GxtEntry* pData = nullptr;
		unsigned int numEntries = 0;

		if (DecompileContent(ifs, &pData, numEntries))
		{
			char szOutput[_MAX_PATH];

			strncpy_s(
				szOutput, 
				szFilePath, 
				strnlen_s(szFilePath, _MAX_PATH) - strnlen_s(szFileExtension + 1 /* skip dot */, _MAX_PATH));

			strcat_s(szOutput, "txt");

			if (SaveDecompiledContent(szOutput, pData, numEntries)) {
				printf("Successfully saved decompiled content.\n");
			}
			else {
				printf("Failed to saved decompiled content!\n");
			}

			delete[] pData;
		}
		else 
		{
			printf("Failed to decompile content!\n");
			return 1;
		}
	}

	if (bCreateMode)
	{
		Map data;

		CompileContent(ifs, data);
		SaveToGxt2(data);
	}
	

	return 0;
}