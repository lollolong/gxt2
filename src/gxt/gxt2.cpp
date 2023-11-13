//
//	gxt/gxt2.cpp
//

// Project
#include "gxt2.h"
#include "main/main_include.h"

// C/C++
#include <format>
#include <fstream>
#include <cstdio>
#include <string.h>
#include <sys/stat.h>

bool DecompileContent(ifstream& stream, GxtEntry** pData, unsigned int& entryCount)
{
	if (!pData) {
		printf("Output for GXT data is invalid.\n");
		return false;
	}

	unsigned int magic = 0, numEntries = 0, dataLength = 0;

	stream.read((char*)&magic,		sizeof(magic));			// Magic
	stream.read((char*)&numEntries, sizeof(numEntries));	// Count

	if (magic != 'GXT2') {
		printf("Not GXT2 file format.\n");
		return false;
	}
	
	entryCount = numEntries;
	*pData = system_new GxtEntry[numEntries];

	for (unsigned int c = 0; c < numEntries; c++)
	{
		stream.read((char*)&(*pData)[c].m_Hash,		sizeof(unsigned int));	// GXT Hash
		stream.read((char*)&(*pData)[c].m_Offset,	sizeof(unsigned int));	// Offset
	}

	stream.read((char*)&magic,		sizeof(magic));			// Magic
	stream.read((char*)&dataLength, sizeof(dataLength));	// File size

	unsigned int stringHeapStart	= static_cast<unsigned int>(stream.tellg());
	unsigned int stringHeapSize		= dataLength - stringHeapStart;

	char* pStringHeap = system_new char[stringHeapSize];
	stream.read(pStringHeap, stringHeapSize);

	for (unsigned int c = 0; c < numEntries; c++)
	{
		const char* szEntry = (pStringHeap + ((*pData)[c].m_Offset - stringHeapStart));
		//printf("0x%08x = %s\n", (*pData)[c].m_Hash, szEntry);
		(*pData)[c].m_Data = szEntry;
	}

	system_delete_array(pStringHeap);
	return true;
}

bool SaveDecompiledContent(const char* szFileName, GxtEntry* pData, unsigned int entryCount)
{
	ofstream ofs(szFileName);

	if (!ofs.is_open()) {
		printf("The output file could not be opened.\n");
		return false;
	}

	for (unsigned int c = 0; c < entryCount; c++)
	{
		ofs << std::format("0x{:08X} = {}", pData[c].m_Hash, pData[c].m_Data) << std::endl;
	}

	return true;
}


//unsigned int magic = 'GXT2';
//unsigned int count = (int)txtDebugMap.size();
//
////std::sort(txtReleaseMap.begin(), txtReleaseMap.end());
////std::sort(txtReleaseMap.begin(), txtReleaseMap.end(), [](const int& a, const int& b) {return a < b; });
//
//gxt2.write((char*)&magic, sizeof(magic));
//gxt2.write((char*)&count, sizeof(count));
//
//unsigned offset = (count * 2 + 4) * 4;
//for (const auto& [uHash, txtEntry] : txtDebugMap)
//{
//	gxt2.write((char*)&uHash, sizeof(uHash));
//	gxt2.write((char*)&offset, sizeof(offset));
//	offset += (unsigned)strlen(txtEntry.c_str()) + 1;
//}
//
//gxt2.write((char*)&magic, sizeof(magic));
//gxt2.write((char*)&offset, sizeof(offset));
//
//for (const auto& [uHash, txtEntry] : txtDebugMap)
//{
//	char null = '\0';
//
//	gxt2.write(txtEntry.c_str(), txtEntry.size());
//	gxt2.write(&null, sizeof(null));
