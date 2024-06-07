//
//	gxt/merge.cpp
//

// Project
#include "merge.h"
#include "main/main.h"

CMerger::CMerger(const std::string& file1, const std::string& file2, const std::string& outfile)
{
	m_Input1 = GXT_NEW CGxt2File(file1, CFile::FLAGS_READ_COMPILED);
	m_Input2 = GXT_NEW CGxt2File(file2, CFile::FLAGS_READ_COMPILED);
	m_Output = GXT_NEW CGxt2File(outfile, CFile::FLAGS_WRITE_COMPILED);

	assert(m_Input1);
	assert(m_Input2);
	assert(m_Output);
} // ::CMerger()

CMerger::~CMerger()
{
	Reset();
} // ::~CMerger()

void CMerger::Reset()
{
	if (m_Input1)
	{
		delete m_Input1;
		m_Input1 = nullptr;
	}
	if (m_Input2)
	{
		delete m_Input2;
		m_Input2 = nullptr;
	}
	if (m_Output)
	{
		delete m_Output;
		m_Output = nullptr;
	}
} // void ::Reset()

bool CMerger::Run()
{
	if (!m_Input1->ReadEntries())
	{
		return false;
	}
	if (!m_Input2->ReadEntries())
	{
		return false;
	}
	m_Output->SetData(m_Input1->GetData());
	m_Output->SetData(m_Input2->GetData());
	return m_Output->WriteEntries();

} // bool ::Run()
