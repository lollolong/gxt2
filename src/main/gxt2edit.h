//
//	main/gxt2edit.h
//

#ifndef _GXT2EDIT_H_
#define _GXT2EDIT_H_

// Project
#include "gxt/gxt2.h"
#include "system/app.h"

// vendor
#include <imgui.h>

// C/C++
#include <vector>

class gxt2edit : public CAppUI
{
public:
	gxt2edit(const string& windowTitle, int width, int height);
	virtual ~gxt2edit();
public:
	int Run(int argc, char* argv[]) override;
private:
	void OnTick() override;

	void Reset();
	void SaveToFile(const string& path);
	void LoadFromFile(const string& path);

	//---------------- Dialogs ----------------
	//
	void OpenFile();
	void SaveFile();
	void SaveFileAs();

	void UpdateEntries();
	void FlagForDeletion(unsigned int uHash);

private:
	//---------------- IO ----------------
	//
	CFile* m_Input;
	CFile* m_Output;
	CFile::Map m_Data;
	string m_Path;

	//---------------- UI ----------------
	//
	ImVec2 m_BarSize;
	string m_HashInput;
	string m_LabelInput;
	string m_TextInput;

	vector<unsigned int> m_EntriesToRemove;
};

#endif // _GXT2EDIT_H_