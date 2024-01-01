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
#include <vector>
#include <string.h>
#include <sys/stat.h>

#include <utility>
#include <algorithm>
#include <functional>

#include <map>

bool CompileContent(ifstream& stream, Map& vData)
{
	string line;

	while (getline(stream, line))
	{
		const string szHash = line.substr(0, 10);
		const string szText = line.substr(13);
		unsigned int uHash	= strtoul(szHash.c_str(), NULL, 16);

		printf("szHash = %s\n", szHash.c_str());
		printf("szHash = %s\n", szText.c_str());

		vData.insert(make_pair(uHash, szText));
	}

	return true;
}

void SaveToGxt2(const Map& vData)
{
	ofstream ofs("tmp.gxt2", std::ios::binary);
	
	unsigned int uCount					= static_cast<unsigned int>(vData.size());
	//unsigned int uOffset				= 4 * (uCount * 2 + 4);	// ((Hash + Offset * uCount) + uMagic + uCount + uMagic + uOffset)
	unsigned uOffset = (uCount * 2 + 4) * 4;
	static const unsigned int uMagic	= 'GXT2';

	printf("uCount = %u\n", uCount);

	////vector<Map::value_type> v(vData.begin(), vData.end());

	//vector items(vData.begin(), vData.end());

	//std::sort(items.begin(), items.end(), [](const auto& a, const auto& b)
	//{
	//	return a.first < b.first;
	//});

	ofs.write((char*)&uMagic, sizeof(uMagic));	// Magic
	ofs.write((char*)&uCount, sizeof(uCount));	// Count

	for (const auto& [uHash, szText] : vData)
	{
		ofs.write((char*)&uHash,	sizeof(uHash));		// GXT Hash
		ofs.write((char*)&uOffset,	sizeof(uOffset));	// Offset

		uOffset += static_cast<unsigned int>(strlen(szText.c_str())) + 1;	// Next Offset
	}

	ofs.write((char*)&uMagic,	sizeof(uMagic));	// Magic
	ofs.write((char*)&uOffset,	sizeof(uOffset));	// File size

	for (const auto& [uHash, szText] : vData)
	{
		ofs.write(szText.c_str(), szText.size());	// String
		ofs.put('\0');								// Null terminator
	}

	ofs.close();
}

bool DecompileContent(ifstream& stream, GxtEntry** pData, unsigned int& entryCount)
{
	if (!pData) {
		printf("Output for GXT data is invalid.\n");
		return false;
	}

	unsigned int magic = 0, numEntries = 0, dataLength = 0;

	stream.setstate(stream.rdstate() | ios::binary);
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