//
//	main/main.cpp
//

// Project
#include "gxt2conv.h"

#include "gxt/gxt2.h"
#include "gxt/convert.h"

// C/C++
#include <fstream>
#include <stdlib.h>
#include <string.h>

int gxt2conv::Run(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("Usage: %s global.gxt2\n\t", argv[0]);
		return 1;
	}

	CConverter gxtConverter(argv[1]);
	gxtConverter.Convert();
	return 0;
}

int main(int argc, char* argv[])
{
	try
	{
		gxt2conv gxt2conv;
		return gxt2conv.Run(argc, argv);
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
//	return 0;
}