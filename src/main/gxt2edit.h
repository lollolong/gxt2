//
//	main/gxt2edit.h
//

#ifndef _GXT2EDIT_H_
#define _GXT2EDIT_H_

// Project
#include "gxt/gxt2.h"
#include "system/app.h"

#include <vector>

class gxt2edit : public CAppUI
{
public:
	gxt2edit(const string& windowTitle, int width, int height);
	virtual ~gxt2edit() = default;
public:
	int Run(int argc, char* argv[]) override;
private:
	void OnTick() override;

	void UpdateEntries();
	void FlagForDeletion(unsigned int uHash);

	CFile* m_Input;
	vector<unsigned int> m_EntriesToRemove;
};

#endif // _GXT2EDIT_H_