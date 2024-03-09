//
//	gxt/convert.h
//

#ifndef _CONVERT_H_
#define _CONVERT_H_

// Project
#include "gxt2.h"

class CConverter
{
public:
	explicit CConverter(const string& filePath);
	virtual ~CConverter();

	void Reset();
	void Convert();

	CFile* GetInput() { return m_Input; }
	const CFile* GetInput() const { return m_Input; }

	CFile* GetOutput() { return m_Output; }
	const CFile* GetOutput() const { return m_Output; }

private:
	void CreateInputInterface(const string& filePath);
	void CreateOutputInterface(const string& filePath);

private:
	CFile* m_Input;
	CFile* m_Output;
};

#endif // _CONVERT_H_