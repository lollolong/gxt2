//
//	gxt/convert.h
//

#ifndef _CONVERT_H_
#define _CONVERT_H_

#include "gxt2.h"

namespace fs = std::filesystem;

class CConverter
{
public:
	explicit CConverter(const string& filePath);
	virtual ~CConverter();

	void Reset();
	void Convert();

private:
	void CreateInputInterface(const string& inputPath);
	void CreateOutputInterface(const string& outputPath);

private:
	CTextFile* m_Input;
	CTextFile* m_Output;

	CTextFile* GetInput() { return m_Input; }
	const CTextFile* GetInput() const { return m_Input; }

	CTextFile* GetOutput() { return m_Output; }
	const CTextFile* GetOutput() const { return m_Output; }
};

#endif // _CONVERT_H_