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
	if (argc < 2 || argc > 3)
	{
		printf("Usage: %s global.gxt2\n\t", argv[0]);
		return 1;
	}

	CConverter gxtConverter(argv[1]);

	if (argc == 3)
	{
		if (strcmp(argv[2], "/le") == 0)
		{
			gxtConverter.GetOutput()->SetLittleEndian();
		}
		if (strcmp(argv[2], "/be") == 0)
		{
			gxtConverter.GetOutput()->SetBigEndian();
		}
	}

	gxtConverter.Convert();
	return 0;
}

gxt2conv& gxt2conv::GetInstance()
{
	static gxt2conv gxt2conv;
	return gxt2conv;
}

int main(int argc, char* argv[])
{
	try
	{
		return gxt2conv::GetInstance().Run(argc, argv);
	}
	catch (const std::exception& ex)
	{
		printf("Error: %s\n", ex.what());
		return 1;
	}
	catch (...)
	{
		printf("Unknown error occurred!\n");
		return 1;
	}
}