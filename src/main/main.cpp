//
//	main/main.cpp
//

// Project
#include "main.h"
#include "gxt/gxt2.h"
#include "gxt/convert.h"

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
		puts("No file extension.\n");
		return 1;
	}

	const bool bAllowedType			= !_stricmp(szFileExtension, ".gxt2") ||
										!_stricmp(szFileExtension, ".txt");

	if (!bAllowedType) {
		puts("Unknown input file format.\n");
		return 1;
	}

	try
	{
		CConverter gxtConverter(szFilePath);
		gxtConverter.Convert();

		printf("Successfully saved content.\n");
	}
	catch (const exception& e)
	{
		printf("Error: %s\n", e.what());
		return 1;
	}

	return 0;
}