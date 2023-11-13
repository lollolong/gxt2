//
//	main/main.cpp
//

// Project
#include "main.h"
#include "gxt/gxt2.h"

// C/C++
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

	FILE* pFile;
	const int err = fopen_s(&pFile, szFilePath, "rb");

	if (err != 0) {
		printf("The specified file could not be opened.\n");
		return 1;
	}

	if (bReadMode)
	{
		GxtEntry* pData = nullptr;
		unsigned int numEntries = 0;

		if (DecompileContent(pFile, &pData, numEntries))
		{
			char szOutput[_MAX_PATH];

			strncpy_s(
				szOutput, 
				szFilePath, 
				strnlen_s(szFilePath, _MAX_PATH) - strnlen_s(szFileExtension + 1 /* skip dot */, _MAX_PATH));

			strcat_s(szOutput, "txt");

			SaveDecompiledContent(szOutput, pData, numEntries);

			delete[] pData;
		}
	}

	if (bCreateMode)
	{

	}
	

	return 0;
}