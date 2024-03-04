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
	if (argc != 2) {
		printf("Usage: %s global.gxt2\n\t", argv[0]);
		return 1;
	}

	const char* szFilePath			= argv[1];
	const char* szFileName			= strrchr(szFilePath, '\\') ? strrchr(szFilePath, '\\') + 1 : szFilePath;
	const char* szFileExtension		= strrchr(szFileName, '.');

	if (!szFileExtension) {
		printf("No file extension.\n");
		return 1;
	}

	const bool bAllowedType =		!_stricmp(szFileExtension, ".gxt2") ||
									!_stricmp(szFileExtension, ".txt");

	const bool bReadMode			= _stricmp(szFileExtension, ".gxt2") == 0;
	const bool bCreateMode			= _stricmp(szFileExtension, ".txt")  == 0;

	if (!bAllowedType) {
		printf("Unknown input file format.\n");
		return 1;
	}

	CTextFile* pInput = nullptr;

	if (bReadMode)
	{
		pInput = new CGxtFile(szFilePath);

		if (pInput->ReadEntries())
		{
			//char szOutput[_MAX_PATH];

			//strncpy_s(
			//	szOutput, 
			//	szFilePath, 
			//	strnlen_s(szFilePath, _MAX_PATH) - strnlen_s(szFileExtension + 1 /* skip dot */, _MAX_PATH));

			//strcat_s(szOutput, "txt");

			//if (SaveDecompiledContent(szOutput, pData, numEntries)) {
			//	printf("Successfully saved decompiled content.\n");
			//}
			//else {
			//	printf("Failed to saved decompiled content!\n");
			//}

			//delete[] pData;

			pInput->Dump();
		}
		else 
		{
			printf("Failed to decompile content!\n");
			return 1;
		}
	}

	if (bCreateMode)
	{
		pInput = new CTxtFile(szFilePath);
	}
	
	if (pInput)
	{
		delete pInput;
	}

	return 0;
}