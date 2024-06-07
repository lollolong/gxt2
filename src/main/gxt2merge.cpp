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
	if (argc < 4 || argc > 5)
	{
		printf("Usage: %s <file1.gxt2> <file2.gxt2> <output.gxt2>\n\t", argv[0]);
		return 1;
	}

	CMerger merger(argv[1], argv[2], argv[3]);

	if (argc == 5)
	{
		if (strcmp(argv[4], "/le") == 0)
		{
			merger.GetOutput()->SetLittleEndian();
		}
		if (strcmp(argv[4], "/be") == 0)
		{
			merger.GetOutput()->SetBigEndian();
		}
	}
	if (!merger.Run())
	{
		printf("Merge failed!\n");
		return 1;
	}
	//printf("Successfully merged!\n");
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