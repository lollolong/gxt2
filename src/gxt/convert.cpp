//
//	gxt/convert.cpp
//

#include "convert.h"

#include <filesystem>

using namespace std;



CConverter::CConverter(const string& filePath) :
	m_Input(nullptr),
	m_Output(nullptr)
{
	CreateInputInterface(filePath);
	CreateOutputInterface(filePath);
}

CConverter::~CConverter()
{
	Reset();
}

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
}

void CConverter::CreateInputInterface(const string& inputPath)
{
	const string szFileExtension = inputPath.substr(inputPath.find_last_of("."));

	if (szFileExtension == ".gxt2")
	{
		m_Input = new CGxtFile(inputPath, CTextFile::FLAGS_READ_COMPILED);
	}
	else if (szFileExtension == ".txt")
	{
		m_Input = new CTxtFile(inputPath, CTextFile::FLAGS_READ_DECOMPILED);
	}
	else
	{
		throw std::invalid_argument("Unknown input file format.");
	}
}

void CConverter::CreateOutputInterface(const string& inputPath)
{
	string szOutputPath = inputPath.substr(0, inputPath.find_last_of("."));
	const string szInputExtension = inputPath.substr(inputPath.find_last_of("."));

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
}

void CConverter::Convert()
{
	if (!GetInput()->ReadEntries()) {
		throw std::runtime_error("Failed to read content.");
	}


	GetOutput()->SetData(GetInput()->GetData());

	if (!GetOutput()->WriteEntries()) {
		throw std::runtime_error("Failed to save content.");
	}
}