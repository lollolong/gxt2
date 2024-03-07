//
//	gxt/gxt2.cpp
//

#include "gxt2.h"
#include <format>


CTextFile::CTextFile(const string& fileName, int openFlags /*= FLAGS_DEFAULT*/)
{
	Reset();
	m_File.open(fileName, openFlags);

	if (!IsOpen())
	{
		printf("The specified file could not be opened.\n");
	}
} // ::CTextFile(const string& fileName, int openFlags = FLAGS_DEFAULT)

CTextFile::~CTextFile()
{
	if (IsOpen())
	{
		m_File.close();
	}
	Reset();
} // ::~CTextFile()

void CTextFile::Reset()
{
	m_Entries.clear();
} // void ::Reset()

void CTextFile::Dump() const
{
	for (const auto& [uHash, szTextEntry] : m_Entries)
	{
		cout << std::format("0x{:08X} = {}", uHash, szTextEntry) << std::endl;
	}
} // void ::Dump() const

bool CTextFile::IsOpen() const
{
	return m_File.is_open();
} // bool ::IsOpen() const

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

const CTextFile::Map& CTextFile::GetData() const
{
	return m_Entries;
} // const CTextFile::Map& ::GetData() const

void CTextFile::SetData(const Map& data)
{
	m_Entries = data;
} // void ::SetData()

//-----------------------------------------------------------------------------------------
//

CGxtFile::CGxtFile(const string& fileName, int openFlags /*= FLAGS_READ_COMPILED*/) :
	CTextFile(fileName, openFlags)
{
} // ::CGxtFile(const string& fileName, int openFlags = FLAGS_READ_COMPILED)

bool CGxtFile::ReadEntries()
{
	if (!IsOpen())
	{
		return false;
	}

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

bool CGxtFile::WriteEntries()
{
	if (!IsOpen())
	{
		return false;
	}

	unsigned int uCount = static_cast<unsigned int>(m_Entries.size());
	unsigned int uOffset = (uCount * 2 + 4) * 4;

	Write(&CGxtFile::GXT2_MAGIC);
	Write(&uCount);

	for (const auto& [uHash, szTextEntry] : m_Entries)
	{
		Write(&uHash);
		Write(&uOffset);

		uOffset += static_cast<unsigned int>(szTextEntry.size()) + 1;
	}

	Write(&CGxtFile::GXT2_MAGIC);
	Write(&uOffset);

	for (const auto& [uHash, szTextEntry] : m_Entries)
	{
		WriteStr(szTextEntry.c_str());
	}

	return true;
} // bool ::WriteEntries()

//-----------------------------------------------------------------------------------------
//

CTxtFile::CTxtFile(const string& fileName, int openFlags /*= FLAGS_READ_DECOMPILED*/) :
	CTextFile(fileName, openFlags)
{
} // ::CTxtFile(const string& fileName, int openFlags = FLAGS_READ_DECOMPILED)

bool CTxtFile::ReadEntries()
{
	if (!IsOpen())
	{
		return false;
	}

	std::string line;

	while (getline(m_File, line))
	{
		const string szHash = line.substr(0, 10);
		const string szText = line.substr(13);
		unsigned int uHash = strtoul(szHash.c_str(), NULL, 16);

		printf("%s\n", szHash.c_str());

		m_Entries[uHash] = szText;
	}
	return true;
} // bool ::ReadEntries()

bool CTxtFile::WriteEntries()
{
	if (!IsOpen())
	{
		return false;
	}

	for (const auto& [uHash, szTextEntry] : m_Entries)
	{
		m_File << std::format("0x{:08X} = {}", uHash, szTextEntry) << std::endl;
	}
	return true;
} // bool ::WriteEntries()