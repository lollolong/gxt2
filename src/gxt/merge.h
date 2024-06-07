//
//	gxt/merge.h
//

#ifndef _MERGE_H_
#define _MERGE_H_

// Project
#include "gxt2.h"

class CMerger
{
public:
	CMerger(const std::string& file1, const std::string& file2, const std::string& outfile);
	virtual ~CMerger();

	void Reset();
	bool Run();

	CFile* GetOutput() { return m_Output; }
	const CFile* GetOutput() const { return m_Output; }
private:
	CFile* m_Input1;
	CFile* m_Input2;
	CFile* m_Output;
};

#endif // !_MERGE_H_