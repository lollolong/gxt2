//
//	gxt/convert.cpp
//

#include "convert.h"

CConverter::CConverter(const string& filePath) :
	m_Input(nullptr),
	m_Output(nullptr)
{
	CreateInputInterface(filePath);
	CreateOutputInterface(filePath);
} // ::CConverter(const string& filePath)

CConverter::~CConverter()
{
	Reset();
} // ::~CConverter()

void CConverter::Reset()
{
	if (m_Input)
	{
		delete m_Input;
		m_Input = nullptr;
	}
	if (m_Output)
	{
		delete m_Output;
		m_Output = nullptr;
	}
} // void ::Reset()

void CConverter::Convert()
{
	if (!GetInput()->ReadEntries())
	{
		throw std::runtime_error("Failed to read content.");
	}

	GetOutput()->SetData(GetInput()->GetData());
	if (!GetOutput()->WriteEntries())
	{
		throw std::runtime_error("Failed to save content.");
	}
} // void ::Convert()

void CConverter::CreateInputInterface(const string& filePath)
{
	const string szFileExtension = filePath.substr(filePath.find_last_of("."));

	if (szFileExtension == ".gxt2")
	{
		m_Input = new CGxtFile(filePath, CTextFile::FLAGS_READ_COMPILED);
	}
	else if (szFileExtension == ".txt")
	{
		m_Input = new CTxtFile(filePath, CTextFile::FLAGS_READ_DECOMPILED);
	}
	else
	{
		throw std::invalid_argument("Unknown input file format.");
	}
} // void ::CreateInputInterface(const string& filePath)

void CConverter::CreateOutputInterface(const string& filePath)
{
	string szOutputPath = filePath.substr(0, filePath.find_last_of("."));
	const string szInputExtension = filePath.substr(filePath.find_last_of("."));

	if (szInputExtension == ".gxt2")
	{
		szOutputPath += ".txt";
		m_Output = new CTxtFile(szOutputPath, CTextFile::FLAGS_WRITE_DECOMPILED);
	}
	else if (szInputExtension == ".txt")
	{
		szOutputPath += ".gxt2";
		m_Output = new CGxtFile(szOutputPath, CTextFile::FLAGS_WRITE_COMPILED);
	}
	else
	{
		throw std::invalid_argument("Unknown input file format.");
	}
} // void ::CreateOutputInterface(const string& filePath)
