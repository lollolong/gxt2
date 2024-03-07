//
//	gxt/gxt2.cpp
//

// Project
#include "gxt2.h"

// vendor
#include <rapidjson/stringbuffer.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>

// C/C++
#include <format>

CFile::CFile(const string& fileName, int openFlags /*= FLAGS_DEFAULT*/)
{
	Reset();
	m_File.open(fileName, openFlags);

	if (!IsOpen())
	{
		throw runtime_error(format("The specified file {} could not be opened.", fileName));
	}
} // ::CFile(const string& fileName, int openFlags = FLAGS_DEFAULT)

CFile::~CFile()
{
	if (IsOpen())
	{
		m_File.close();
	}
	Reset();
} // ::~CFile()

CFile& CFile::operator=(const CFile& other)
{
	this->m_Entries = other.m_Entries;
	return *this;
} // CFile& ::operator=(const CFile& other)

void CFile::Reset()
{
	m_Entries.clear();
} // void ::Reset()

void CFile::Dump() const
{
	for (const auto& [uHash, szTextEntry] : m_Entries)
	{
		cout << std::format("0x{:08X} = {}", uHash, szTextEntry) << std::endl;
	}
} // void ::Dump() const

bool CFile::IsOpen() const
{
	return m_File.is_open();
} // bool ::IsOpen() const

void CFile::Head()
{
	m_File.seekg(0, ios::beg);
} // void ::Head()

void CFile::End()
{
	m_File.seekg(0, ios::end);
} // void ::End()

void CFile::Seek(int cursor)
{
	m_File.seekg(cursor);
} // void ::Seek(int cursor)

unsigned int CFile::GetPosition()
{
	return static_cast<unsigned int>(m_File.tellg());
} // unsigned int ::GetPosition()

const CFile::Map& CFile::GetData() const
{
	return m_Entries;
} // const CFile::Map& ::GetData() const

void CFile::SetData(const Map& data)
{
	m_Entries = data;
} // void ::SetData()

//-----------------------------------------------------------------------------------------
//

CGxt2File::CGxt2File(const string& fileName, int openFlags /*= FLAGS_READ_COMPILED*/) :
	CFile(fileName, openFlags)
{
} // ::CGxt2File(const string& fileName, int openFlags = FLAGS_READ_COMPILED)

bool CGxt2File::ReadEntries()
{
	if (!IsOpen())
	{
		return false;
	}

	unsigned int uMagic = 0, uNumEntries = 0, uDataLength = 0;

	Head();
	Read(&uMagic);
	Read(&uNumEntries);

	if (uMagic != CGxt2File::GXT2_MAGIC)
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

	const unsigned int uHeapStart = GetPosition();
	const unsigned int uHeapLength = uDataLength - uHeapStart;

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

bool CGxt2File::WriteEntries()
{
	if (!IsOpen())
	{
		return false;
	}

	unsigned int uCount = static_cast<unsigned int>(m_Entries.size());
	unsigned int uOffset = (uCount * 2 + 4) * 4;

	Write(&CGxt2File::GXT2_MAGIC);
	Write(&uCount);

	for (const auto& [uHash, szTextEntry] : m_Entries)
	{
		Write(&uHash);
		Write(&uOffset);

		uOffset += static_cast<unsigned int>(szTextEntry.size()) + 1;
	}

	Write(&CGxt2File::GXT2_MAGIC);
	Write(&uOffset);

	for (const auto& [uHash, szTextEntry] : m_Entries)
	{
		WriteStr(szTextEntry.c_str());
	}

	return true;
} // bool ::WriteEntries()

//-----------------------------------------------------------------------------------------
//

CTextFile::CTextFile(const string& fileName, int openFlags /*= FLAGS_READ_DECOMPILED*/) :
	CFile(fileName, openFlags)
{
} // ::CTextFile(const string& fileName, int openFlags = FLAGS_READ_DECOMPILED)

bool CTextFile::ReadEntries()
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

bool CTextFile::WriteEntries()
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

//-----------------------------------------------------------------------------------------
//

CJsonFile::CJsonFile(const string& fileName, int openFlags /*= FLAGS_READ_DECOMPILED*/) :
	CFile(fileName, openFlags)
{
} // ::CJsonFile(const string& fileName, int openFlags = FLAGS_READ_DECOMPILED)

bool CJsonFile::ReadEntries()
{
	if (!IsOpen())
	{
		return false;
	}

	rapidjson::IStreamWrapper isw(m_File);
	m_Document.ParseStream(isw);

	for (auto it = m_Document.MemberBegin(); it != m_Document.MemberEnd(); it++)
	{
		unsigned int uHash = strtoul(it->name.GetString(), NULL, 16);
		m_Entries[uHash] = it->value.GetString();
	}

	return true;
} // bool ::ReadEntries()

bool CJsonFile::WriteEntries()
{
	if (!IsOpen())
	{
		return false;
	}

	m_Document.SetObject();

	for (const auto& [uHash, szTextEntry] : m_Entries)
	{
		const string szHash = std::format("0x{:08X}", uHash);

		rapidjson::Value key(szHash.c_str(), m_Document.GetAllocator());
		rapidjson::Value value(szTextEntry.c_str(), m_Document.GetAllocator());

		m_Document.AddMember(key, value, m_Document.GetAllocator());
	}

	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
	m_Document.Accept(writer);

	m_File << buffer.GetString();

	return true;
} // bool ::WriteEntries()