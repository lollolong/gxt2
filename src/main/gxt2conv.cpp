//
//	main/main.cpp
//

// Project
#include "gxt/gxt2.h"
#include "gxt/convert.h"

// C/C++
#include <fstream>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("Usage: %s global.gxt2\n\t", argv[0]);
		return 1;
	}

	try
	{
		CConverter gxtConverter(argv[1]);
		gxtConverter.Convert();

		printf("Successfully converted content.\n");
	}
	catch (const exception& ex)
	{
		printf("Error: %s\n", ex.what());
		return 1;
	}
	catch (...)
	{
		printf("Unknown error occurred!\n");
		return 1;
	}
	return 0;
}