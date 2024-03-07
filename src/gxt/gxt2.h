//
//	gxt/gxt2.h
//

#ifndef _GXT2_H_
#define _GXT2_H_

#include <map>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

class CTextFile
{
public:
	enum
	{
		// reading
		FLAGS_READ_COMPILED = (fstream::in | fstream::binary),
		FLAGS_READ_DECOMPILED = (fstream::in),

		// writing
		FLAGS_WRITE_COMPILED = (fstream::out | fstream::binary),
		FLAGS_WRITE_DECOMPILED = (fstream::out),

		// default
		FLAGS_DEFAULT = (fstream::in | fstream::out)
	};
	using Map = map<unsigned int, string, less<unsigned int>>; // do not touch
public:
	CTextFile(const string& fileName, int openFlags = FLAGS_DEFAULT);
	virtual ~CTextFile();

	void Reset();
	void Dump() const;
	bool IsOpen() const;

	const Map& GetData() const;
	void SetData(const Map& data);

	virtual bool ReadEntries() = 0;
	virtual bool WriteEntries() = 0;
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
	fstream m_File;
	Map m_Entries;
};

//-----------------------------------------------------------------------------------------
//

class CGxtFile : public CTextFile
{
public:
	struct Entry
	{
		unsigned int m_Hash;
		unsigned int m_Offset;
	};
public:
	CGxtFile(const string& fileName, int openFlags = FLAGS_READ_COMPILED);

	bool ReadEntries() override;
	bool WriteEntries() override;

	static constexpr unsigned int GXT2_MAGIC = 'GXT2';
};

//-----------------------------------------------------------------------------------------
//

class CTxtFile : public CTextFile
{
public:
	CTxtFile(const string& fileName, int openFlags = FLAGS_READ_DECOMPILED);

	bool ReadEntries() override;
	bool WriteEntries() override;
};

#endif // _GXT2_H_