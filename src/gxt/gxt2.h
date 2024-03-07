//
//	gxt/gxt2.h
//

#ifndef _GXT2_H_
#define _GXT2_H_

#include <map>
#include <fstream>
#include <string>
#include <cstdio>

#include <iostream>
#include <fstream>
#include <iomanip>
#include <unordered_map>
#include <vector>

using namespace std;

class CTextFile
{
public:
	using Map = map<unsigned int, string>;

	CTextFile(const string& fileName);
	CTextFile(const string& fileName, int openFlags);
	virtual ~CTextFile();

	void Reset();
	void Dump() const;
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
protected:
	fstream m_File;
	Map m_Entries;
};

class CGxtFile : public CTextFile
{
public:
	struct Entry
	{
		unsigned int m_Hash;
		unsigned int m_Offset;
	};
public:
	CGxtFile(const string& fileName);

	bool ReadEntries() override;
	bool WriteEntries() override;

	static constexpr unsigned int GXT2_MAGIC = 'GXT2';
};

class CTxtFile : public CTextFile
{
public:
	CTxtFile(const string& fileName);

	bool ReadEntries() override;
	bool WriteEntries() override;
};


#endif // _GXT2_H_