//
//	gxt/gxt2.h
//

#ifndef _GXT2_H_
#define _GXT2_H_

// C/C++
#include <map>
#include <vector>
#include <string>
#include <cstring>
#include <fstream>
#include <iostream>

// vendor
#include <nlohmann/json_fwd.hpp>
#include <nlohmann/json.hpp>

#define MAKE_MAGIC(a, b, c, d)            \
	(static_cast<unsigned int>(a) << 24 | \
	 static_cast<unsigned int>(b) << 16 | \
	 static_cast<unsigned int>(c) << 8  | \
	 static_cast<unsigned int>(d))

class CFile
{
public:
	enum
	{
		// reading
		FLAGS_READ_COMPILED = (std::fstream::in | std::fstream::binary),
		FLAGS_READ_DECOMPILED = (std::fstream::in),

		// writing
		FLAGS_WRITE_COMPILED = (std::fstream::out | std::fstream::binary),
		FLAGS_WRITE_DECOMPILED = (std::fstream::out),

		// default
		FLAGS_DEFAULT = (std::fstream::in | std::fstream::out)
	};
	enum
	{
		_ENDIAN_UNKNOWN,

		_LITTLE_ENDIAN,
		_BIG_ENDIAN
	};
	using Map = std::map<unsigned int, std::string, std::less<unsigned int>>;
	using Vec = std::vector<std::pair<unsigned int, std::string>>;
protected:
	CFile();
public:
	CFile(const std::string& fileName, int openFlags = FLAGS_DEFAULT, int endian = _LITTLE_ENDIAN);
	virtual ~CFile();

	CFile(const CFile&) = delete;
	CFile& operator=(const CFile& other);

	void Reset();
	void Dump() const;
	bool IsOpen() const;
	void Close();

	Map& GetData();
	const Map& GetData() const;
	const Map& GetDataConst() const;
	void SetData(const Map& data);
	void SetData(const Vec& data);

	void SetEndian(int endian) { m_Endian = endian; }
	void SetLittleEndian() { m_Endian = _LITTLE_ENDIAN; }
	void SetBigEndian() { m_Endian = _BIG_ENDIAN; }
	bool IsLittleEndian() const { return m_Endian == _LITTLE_ENDIAN; }
	bool IsBigEndian() const { return m_Endian == _BIG_ENDIAN; }
	int GetEndian() const { return m_Endian; }

	static void SwapEndian(unsigned int& x);
	void DoSwapEndian(unsigned int& x) const;

	virtual bool ReadEntries() { return false; };
	virtual bool WriteEntries() { return false; };
protected:
	void Head();
	void End();
	void Seek(int cursor);
	unsigned int GetPosition();

	template<typename T>
	void Read(T* pData)
	{
		m_File.read(reinterpret_cast<char*>(pData), sizeof(T));
	}

	template<typename T>
	void Read(T* pData, unsigned int size)
	{
		m_File.read(reinterpret_cast<char*>(pData), static_cast<size_t>(size));
	}

	template<typename T>
	void Write(const T* pData)
	{
		m_File.write(reinterpret_cast<const char*>(pData), sizeof(T));
	}

	template<typename T>
	void Write(const T* pData, unsigned int size)
	{
		m_File.write(reinterpret_cast<const char*>(pData), static_cast<size_t>(size));
	}

	void WriteStr(const char* pData)
	{
		m_File.write(pData, strlen(pData) + 1);
	}
protected:
	std::fstream m_File;
	Map m_Entries;
	int m_Endian;
};

//-----------------------------------------------------------------------------------------
//

class CMemoryFile : public CFile
{
public:
	CMemoryFile();

	bool ReadEntries() override;
	bool WriteEntries() override;
};

//-----------------------------------------------------------------------------------------
//

class CGxt2File : public CFile
{
private:
	struct Entry
	{
		unsigned int m_Hash;
		unsigned int m_Offset;
	};
public:
	CGxt2File(const std::string& fileName, int openFlags = FLAGS_READ_COMPILED, int endian = _LITTLE_ENDIAN);

	bool ReadEntries() override;
	bool WriteEntries() override;

	static constexpr unsigned int GXT2_MAGIC_LE = MAKE_MAGIC('G', 'X', 'T', '2');
	static constexpr unsigned int GXT2_MAGIC_BE = MAKE_MAGIC('2', 'T', 'X', 'G');
};

//-----------------------------------------------------------------------------------------
//

class CTextFile : public CFile
{
public:
	CTextFile(const std::string& fileName, int openFlags = FLAGS_READ_DECOMPILED);

	bool ReadEntries() override;
	bool WriteEntries() override;
};

//-----------------------------------------------------------------------------------------
//

class CJsonFile : public CFile
{
public:
	CJsonFile(const std::string& fileName, int openFlags = FLAGS_READ_DECOMPILED);

	bool ReadEntries() override;
	bool WriteEntries() override;
private:
	nlohmann::json m_Data;
};

//-----------------------------------------------------------------------------------------
//

class CCsvFile : public CFile
{
public:
	CCsvFile(const std::string& fileName, int openFlags = FLAGS_READ_DECOMPILED);

	bool ReadEntries() override;
	bool WriteEntries() override;
};

//-----------------------------------------------------------------------------------------
//

class COxtFile : public CFile
{
public:
	COxtFile(const std::string& fileName, int openFlags = FLAGS_READ_DECOMPILED);

	bool ReadEntries() override;
	bool WriteEntries() override;
};

//-----------------------------------------------------------------------------------------
//

class CHashDatabase : public CFile
{
public:
	CHashDatabase(const std::string& fileName, int openFlags = FLAGS_READ_DECOMPILED);

	bool ReadEntries() override;
	bool WriteEntries() override;
};

#endif // !_GXT2_H_