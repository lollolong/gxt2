//
//	main/gxt2edit.h
//

#ifndef _GXT2EDIT_H_
#define _GXT2EDIT_H_

// Project
#include "gxt/gxt2.h"
#include "system/app.h"

class gxt2edit : public CAppUI
{
public:
	gxt2edit(const string& windowTitle);
	virtual ~gxt2edit() = default;
public:
	int Run(int argc, char* argv[]) override;
private:
	void OnTick() override;

	CFile* m_Input;
};

#endif // _GXT2EDIT_H_