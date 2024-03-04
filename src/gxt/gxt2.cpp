//
//	gxt/gxt2.cpp
//

#include "gxt2.h"
#include <algorithm>
#include <format>

using namespace std;

CTextFile::CTextFile(const string& fileName) :
	CTextFile(fileName, fstream::in | fstream::out)
{
} // ::CTextFile(const string& fileName)

CTextFile::CTextFile(const string& fileName, int openFlags)
{
	Reset();
	m_File.open(fileName, openFlags);

	if (!m_File.is_open())
	{
		printf("The specified file could not be opened.\n");
	}

} // ::CTextFile(const string& fileName, int openFlags)

CTextFile::~CTextFile()
{
	if (m_File.is_open())
	{
		m_File.close();
	}
	Reset();
} // ::~CTextFile()

void CTextFile::Reset()
{
	m_Entries.clear();
} // void ::Reset()

void CTextFile::Head()
{
	m_File.seekg(0, ios::beg);
} // void ::Head()

void CTextFile::End()
{
	m_File.seekg(0, ios::end);
} // void ::End()

void CTextFile::Seek(int cursor)
{
	m_File.seekg(cursor);
} // void ::Seek(int cursor)

unsigned int CTextFile::GetPosition()
{
	return static_cast<unsigned int>(m_File.tellg());
} // unsigned int ::GetPosition()

void CTextFile::Dump() const
{
	for (const auto& [uHash, szTextEntry] : m_Entries)
	{
		cout << std::format("0x{:08X} = {}", uHash, szTextEntry) << std::endl;
	}
} // void ::Dump() const


CGxtFile::CGxtFile(const string& fileName) :
	CTextFile(fileName, fstream::in | fstream::out | fstream::binary)
{
} // ::CGxtFile(const string& fileName)

bool CGxtFile::ReadEntries()
{
	unsigned int uMagic = 0, uNumEntries = 0, uDataLength = 0;

	Head();
	Read(&uMagic);
	Read(&uNumEntries);

	if (uMagic != CGxtFile::GXT2_MAGIC)
	{
		cerr << "Error: Not GXT2 file format." << endl;
		return false;
	}

	Entry* pEntries = new Entry[uNumEntries];
	for (unsigned int uEntry = 0; uEntry < uNumEntries; uEntry++)
	{
		Read(&pEntries[uEntry].m_Hash);
		Read(&pEntries[uEntry].m_Offset);
	}
	Read(&uMagic);
	Read(&uDataLength);

	const unsigned int uHeapStart	= GetPosition();
	const unsigned int uHeapLength	= uDataLength - uHeapStart;

	char* pStringHeap = new char[uHeapLength];
	Read(pStringHeap, uHeapLength);

	for (unsigned int uEntry = 0; uEntry < uNumEntries; uEntry++)
	{
		const char* szTextEntry = pStringHeap + (pEntries[uEntry].m_Offset - uHeapStart);
		m_Entries[pEntries[uEntry].m_Hash] = szTextEntry;
	}

	delete[] pEntries;
	delete[] pStringHeap;

	return true;

} // bool ::ReadEntries()

bool CGxtFile::WriteEntries() const
{
	return false;
} // bool ::WriteEntries() const


CTxtFile::CTxtFile(const string& fileName) :
	CTextFile(fileName)
{
} // ::CTxtFile(const string& fileName)

bool CTxtFile::ReadEntries()
{
	return false;
} // bool ::ReadEntries()

bool CTxtFile::WriteEntries() const
{
	return false;
} // bool ::WriteEntries() const