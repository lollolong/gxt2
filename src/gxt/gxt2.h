//
//	gxt/gxt2.h
//

#ifndef _GXT2_H_
#define _GXT2_H_

// vendor
#include <rapidjson/document.h>

// C/C++
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

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
	using Map = std::map<unsigned int, std::string, std::less<unsigned int>>;
	using Vec = std::vector<std::pair<unsigned int, std::string>>;
public:
	CFile();
	CFile(const std::string& fileName, int openFlags = FLAGS_DEFAULT);
	virtual ~CFile();

	CFile(const CFile&) = delete;
	CFile& operator=(const CFile& other);

	void Reset();
	void Dump() const;
	bool IsOpen() const;

	Map& GetData();
	const Map& GetData() const;
	void SetData(const Map& data);
	void SetData(const Vec& data);

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
};

//-----------------------------------------------------------------------------------------
//

class CGxt2File : public CFile
{
public:
	struct Entry
	{
		unsigned int m_Hash;
		unsigned int m_Offset;
	};
public:
	CGxt2File(const std::string& fileName, int openFlags = FLAGS_READ_COMPILED);

	bool ReadEntries() override;
	bool WriteEntries() override;

	static constexpr unsigned int GXT2_MAGIC = 'GXT2';
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
	rapidjson::Document m_Document;
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

#endif // _GXT2_H_