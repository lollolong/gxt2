//
//	main/gxt2merge.cpp
//

// Project
#include "gxt2merge.h"

#include "gxt/gxt2.h"
#include "gxt/merge.h"

// C/C++
#include <fstream>
#include <stdlib.h>
#include <string.h>

int gxt2merge::Run(int argc, char* argv[])
{
	if (argc < 2 || argc > 3)
	{
		printf("Usage: %s global.gxt2\n\t", argv[0]);
		return 1;
	}
	return 0;
}

gxt2merge& gxt2merge::GetInstance()
{
	static gxt2merge gxt2merge;
	return gxt2merge;
}

int main(int argc, char* argv[])
{
	try
	{
		return gxt2merge::GetInstance().Run(argc, argv);
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