//
//	gxt/gxt2.cpp
//

// Project
#include "gxt2.h"
#include "main/main_include.h"

// C/C++
#include <cstdio>
#include <string.h>
#include <sys/stat.h>

bool DecompileContent(FILE* pFile, GxtEntry** pData, unsigned int& entryCount)
{
	if (!pFile) {
		printf("Cannot read GXT2 from invalid stream.\n");
		return false;
	}

	if (!pData) {
		printf("Output for GXT data is invalid.\n");
		return false;
	}

	unsigned int magic = 0, numEntries = 0, dataLength = 0;
	fread_s(&magic,			sizeof(magic),		sizeof(magic),		1, pFile);	// Magic
	fread_s(&numEntries,	sizeof(numEntries), sizeof(numEntries), 1, pFile);	// Count

	if (magic != 'GXT2') {
		printf("Not GXT2 file format.\n");
		return false;
	}
	
	entryCount = numEntries;
	*pData = system_new GxtEntry[numEntries];

	for (unsigned int c = 0; c < numEntries; c++)
	{
		fread_s(&(*pData)[c].m_Hash,	sizeof(unsigned int), sizeof(unsigned int), 1, pFile);	// GXT Hash
		fread_s(&(*pData)[c].m_Offset,	sizeof(unsigned int), sizeof(unsigned int), 1, pFile);	// Offset
	}

	fread_s(&magic,			sizeof(magic),		sizeof(magic),		1, pFile);	// Magic
	fread_s(&dataLength,	sizeof(dataLength), sizeof(dataLength), 1, pFile);	// File size

	unsigned int stringHeapStart	= ftell(pFile);
	unsigned int stringHeapSize		= dataLength - stringHeapStart;

	char* pStringHeap = system_new char[stringHeapSize];
	fread_s(pStringHeap, stringHeapSize, stringHeapSize, 1, pFile);

	for (unsigned int c = 0; c < numEntries; c++)
	{
		const char* szEntry = (pStringHeap + ((*pData)[c].m_Offset - stringHeapStart));
		//printf("0x%08x = %s\n", (*pData)[c].m_Hash, szEntry);
		(*pData)[c].m_Data = szEntry;
	}

	system_delete_array(pStringHeap);
	//system_delete_array(*pData);
	return true;
}

bool SaveDecompiledContent(const char* szFileName, GxtEntry* pData, unsigned int entryCount)
{
	FILE* pFile;
	const int err = fopen_s(&pFile, szFileName, "w");

	if (err != 0) {
		printf("The output file could not be opened.\n");
		return err;
	}

	for (unsigned int c = 0; c < entryCount; c++)
	{
		char szLine[1024];


		sprintf_s(szLine, "0x%08X = ", pData[c].m_Hash);
		fwrite(szLine, strlen(szLine), 1, pFile);
		fwrite(pData[c].m_Data.c_str(), pData[c].m_Data.size(), 1, pFile);
		fputc('\n', pFile);
	}

	fclose(pFile);
	pFile = nullptr;

	return true;
}