//
//	gxt/convert.h
//

#ifndef _CONVERT_H_
#define _CONVERT_H_

#include "gxt2.h"

class CConverter
{
public:
	explicit CConverter(const string& filePath);
	virtual ~CConverter();

	void Reset();
	void Convert();

private:
	void CreateInputInterface(const string& filePath);
	void CreateOutputInterface(const string& filePath);

private:
	CTextFile* m_Input;
	CTextFile* m_Output;

	CTextFile* GetInput() { return m_Input; }
	const CTextFile* GetInput() const { return m_Input; }

	CTextFile* GetOutput() { return m_Output; }
	const CTextFile* GetOutput() const { return m_Output; }
};

#endif // _CONVERT_H_