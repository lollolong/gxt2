//
//	gxt/gxt2.h
//

#ifndef _GXT2_H_
#define _GXT2_H_

#include <fstream>
#include <string>
#include <cstdio>

#include <map>

using namespace std;

struct GxtEntry
{
	unsigned int m_Hash;
	unsigned int m_Offset;
	string m_Data;
};

typedef map<unsigned int, string/*, less<int>*/> Map;

bool CompileContent(ifstream& stream, Map& vData);
void SaveToGxt2(const Map& vData);

bool DecompileContent(ifstream& stream, GxtEntry** pData, unsigned int& entryCount);
bool SaveDecompiledContent(const char* szFileName, GxtEntry* pData, unsigned int entryCount);

//bool CompileContent();

#endif // _GXT2_H_