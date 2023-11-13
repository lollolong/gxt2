//
//	gxt/gxt2.h
//

#ifndef _GXT2_H_
#define _GXT2_H_

#include <string>
#include <cstdio>

using namespace std;

struct GxtEntry
{
	unsigned int m_Hash;
	unsigned int m_Offset;
	string m_Data;
};

bool DecompileContent(FILE* pFile, GxtEntry** pData, unsigned int& entryCount);
bool SaveDecompiledContent(const char* szFileName, GxtEntry* pData, unsigned int entryCount);

#endif // _GXT2_H_