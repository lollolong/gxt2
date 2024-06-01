//
//	gxt/gxt2.cpp
//

// Project
#include "gxt2.h"
#include "main/main.h"
#include "data/stringhash.h"

// vendor
#include <fstream>
#include <ios>

// C/C++
#include <format>

CFile::CFile()
{
	Reset();
} // ::CFile()

CFile::CFile(const std::string& fileName, int openFlags /*= FLAGS_DEFAULT*/, int endian /*= LITTLE_ENDIAN*/)
	: m_Endian(endian)
{
	Reset();
	m_File.open(fileName, static_cast<std::ios_base::openmode>(openFlags));

	if (!IsOpen())
	{
		throw std::runtime_error(std::format("The specified file {} could not be opened.", fileName));
	}
} // ::CFile(const string& fileName, int openFlags = FLAGS_DEFAULT, int endian = LITTLE_ENDIAN)

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
		std::cout << std::format("0x{:08X} = {}", uHash, szTextEntry) << std::endl;
	}
} // void ::Dump() const

bool CFile::IsOpen() const
{
	return m_File.is_open();
} // bool ::IsOpen() const

void CFile::Head()
{
	m_File.seekg(0, std::ios::beg);
} // void ::Head()

void CFile::End()
{
	m_File.seekg(0, std::ios::end);
} // void ::End()

void CFile::Seek(int cursor)
{
	m_File.seekg(cursor);
} // void ::Seek(int cursor)

unsigned int CFile::GetPosition()
{
	return static_cast<unsigned int>(m_File.tellg());
} // unsigned int ::GetPosition()

CFile::Map& CFile::GetData()
{
	return m_Entries;
} // CFile::Map& ::GetData()

const CFile::Map& CFile::GetData() const
{
	return m_Entries;
} // const CFile::Map& ::GetData() const

void CFile::SetData(const Map& data)
{
	m_Entries = data;
} // void ::SetData()

void CFile::SetData(const Vec& data)
{
	Reset();
	m_Entries.insert(data.begin(), data.end());
} // void ::SetData()

void CFile::SwapEndian(unsigned int& x)
{
	x = ((x >> 0x18) & 0x000000FF) |
		((x >> 0x08) & 0x0000FF00) |
		((x << 0x08) & 0x00FF0000) |
		((x << 0x18) & 0xFF000000);
} // void ::SwapEndian(unsigned int& x)

void CFile::DoSwapEndian(unsigned int& x) const
{
	if (IsBigEndian())
	{
		CFile::SwapEndian(x);
	}
} // void ::CheckDoSwapEndian(unsigned int& x)

//-----------------------------------------------------------------------------------------
//

CGxt2File::CGxt2File(const std::string& fileName, int openFlags /*= FLAGS_READ_COMPILED*/, int endian /*= LITTLE_ENDIAN*/) :
	CFile(fileName, openFlags, endian)
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

	if (uMagic == CGxt2File::GXT2_MAGIC_LE)
	{
		SetLittleEndian();
	}
	else if (uMagic == CGxt2File::GXT2_MAGIC_BE)
	{
		SetBigEndian();
	}
	else
	{
		std::cerr << "Error: Not GXT2 file format." << std::endl;
		return false;
	}

	DoSwapEndian(uNumEntries);

	Entry* pEntries = GXT_NEW Entry[uNumEntries];
	for (unsigned int uEntry = 0; uEntry < uNumEntries; uEntry++)
	{
		Read(&pEntries[uEntry].m_Hash);
		Read(&pEntries[uEntry].m_Offset);
		DoSwapEndian(pEntries[uEntry].m_Hash);
		DoSwapEndian(pEntries[uEntry].m_Offset);
	}

	Read(&uMagic);
	Read(&uDataLength);
	DoSwapEndian(uMagic);
	DoSwapEndian(uDataLength);

	if (uMagic != CGxt2File::GXT2_MAGIC_LE && uMagic != CGxt2File::GXT2_MAGIC_BE)
	{
		std::cerr << "Expected GXT2 Magic, your file might be corrupted!" << std::endl;
#if _DEBUG
		__debugbreak();
#endif
	}

	const unsigned int uHeapStart = GetPosition();
	const unsigned int uHeapLength = uDataLength - uHeapStart;

	char* pStringHeap = GXT_NEW char[uHeapLength];
	Read(pStringHeap, uHeapLength);

	for (unsigned int uEntry = 0; uEntry < uNumEntries; uEntry++)
	{
		const char* szTextEntry = pStringHeap + (pEntries[uEntry].m_Offset - uHeapStart);
#if _DEBUG
		if (auto it = m_Entries.find(pEntries[uEntry].m_Hash); it != m_Entries.end())
		{
			if (strcmp(it->second.c_str(), szTextEntry) == 0)
			{
				std::cout << std::format("[{}] Warning: Duplicate Text Entry (0x{:08X}) with same content found!", __FUNCTION__, pEntries[uEntry].m_Hash) << std::endl;
			}
			else
			{
				std::cout << std::format("[{}] Warning: Duplicate Text Entry (0x{:08X}) found!\n\tprv = {}\n\tcur = {}", __FUNCTION__, pEntries[uEntry].m_Hash, it->second, szTextEntry) << std::endl;
			}
			//__debugbreak();
		}
		else
		{
			m_Entries[pEntries[uEntry].m_Hash] = szTextEntry;
		}
#else
		m_Entries[pEntries[uEntry].m_Hash] = szTextEntry;
#endif
	}

	delete[] pEntries;
	delete[] pStringHeap;

#if _DEBUG && 0
	assert(uNumEntries == m_Entries.size());
#endif

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

	if (IsLittleEndian())
	{
		Write(&CGxt2File::GXT2_MAGIC_LE);
	}
	else if (IsBigEndian())
	{
		Write(&CGxt2File::GXT2_MAGIC_BE);
	}

	DoSwapEndian(uCount);
	Write(&uCount);

	for (const auto& [uHash, szTextEntry] : m_Entries)
	{
		unsigned int uHashSwapped = uHash;
		unsigned int uOffsetSwapped = uOffset;

		DoSwapEndian(uHashSwapped);
		DoSwapEndian(uOffsetSwapped);
		Write(&uHashSwapped);
		Write(&uOffsetSwapped);

		uOffset += static_cast<unsigned int>(szTextEntry.size()) + 1;
	}

	if (IsLittleEndian())
	{
		Write(&CGxt2File::GXT2_MAGIC_LE);
	}
	else if (IsBigEndian())
	{
		Write(&CGxt2File::GXT2_MAGIC_BE);
	}
	DoSwapEndian(uOffset);
	Write(&uOffset);

	for (const auto& [uHash, szTextEntry] : m_Entries)
	{
		WriteStr(szTextEntry.c_str());
	}

	return true;
} // bool ::WriteEntries()

//-----------------------------------------------------------------------------------------
//

CTextFile::CTextFile(const std::string& fileName, int openFlags /*= FLAGS_READ_DECOMPILED*/) :
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
	while (std::getline(m_File, line))
	{
		const std::string szHash = line.substr(0, 10);
		const std::string szText = line.substr(13);
		const unsigned int uHash = strtoul(szHash.c_str(), NULL, 16);

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

CJsonFile::CJsonFile(const std::string& fileName, int openFlags /*= FLAGS_READ_DECOMPILED*/) :
	CFile(fileName, openFlags),
	m_Data()
{
} // ::CJsonFile(const string& fileName, int openFlags = FLAGS_READ_DECOMPILED)

bool CJsonFile::ReadEntries()
{
	if (!IsOpen())
	{
		return false;
	}

	m_Data.clear();
	m_File >> m_Data;

	for (auto [key, val] : m_Data.items())
	{
		const unsigned int uHash = strtoul(key.c_str(), NULL, 16);
		m_Entries[uHash] = val.get<std::string>();
	}
	return true;
} // bool ::ReadEntries()

bool CJsonFile::WriteEntries()
{
	if (!IsOpen())
	{
		return false;
	}

	m_Data.clear();

	for (const auto& [uHash, szTextEntry] : m_Entries)
	{
		const std::string szHash = std::format("0x{:08X}", uHash);

		m_Data[szHash] = szTextEntry;
	}

	m_File << m_Data.dump(1);

	return true;
} // bool ::WriteEntries()

//-----------------------------------------------------------------------------------------
//

CCsvFile::CCsvFile(const std::string& fileName, int openFlags /*= FLAGS_READ_DECOMPILED*/) :
	CFile(fileName, openFlags)
{
} // ::CCsvFile(const string& fileName, int openFlags = FLAGS_READ_DECOMPILED)

bool CCsvFile::ReadEntries()
{
	if (!IsOpen())
	{
		return false;
	}

	std::string line;
	while (std::getline(m_File, line))
	{
		const std::string szHash = line.substr(0, 10);
		const std::string szText = line.substr(11);
		const unsigned int uHash = strtoul(szHash.c_str(), NULL, 16);

		m_Entries[uHash] = szText;
	}
	return true;
} // bool ::ReadEntries()

bool CCsvFile::WriteEntries()
{
	if (!IsOpen())
	{
		return false;
	}

	for (const auto& [uHash, szTextEntry] : m_Entries)
	{
		m_File << format("0x{:08X},{}", uHash, szTextEntry) << std::endl;
	}
	return true;
} // bool ::WriteEntries()

//-----------------------------------------------------------------------------------------
//

COxtFile::COxtFile(const std::string& fileName, int openFlags /*= FLAGS_READ_DECOMPILED*/) :
	CFile(fileName, openFlags)
{
} // ::COxtFile(const string& fileName, int openFlags = FLAGS_READ_DECOMPILED)

bool COxtFile::ReadEntries()
{
	if (!IsOpen())
	{
		return false;
	}

	std::string line;
	while (std::getline(m_File, line))
	{
		if (line == "Version 2 30" || line == "{" || line == "}")
			continue;

		const size_t n1 = line.find_first_of('\t');
		const size_t n2 = line.find_first_of('=');
		if (n1 != std::string::npos && n2 != std::string::npos)
		{
			const std::string szHash = line.substr(n1 + 1, n2 - 2);
			const std::string szText = line.substr(n2 + 2);

			if (szHash.starts_with("0x"))
			{
				const unsigned int uHash = strtoul(szHash.c_str(), NULL, 16);
				m_Entries[uHash] = szText;
			}
			else
			{
				m_Entries[rage::atStringHash(szHash.c_str())] = szText;
			}
		}
	}
	return true;
} // bool ::ReadEntries()

bool COxtFile::WriteEntries()
{
	if (!IsOpen())
	{
		return false;
	}

	m_File << "Version 2 30" << std::endl << "{" << std::endl;
	for (const auto& [uHash, szTextEntry] : m_Entries)
	{
		m_File << std::format("\t0x{:08X} = {}", uHash, szTextEntry) << std::endl;
	}
	m_File << "}" << std::endl;

	return true;
} // bool ::WriteEntries()