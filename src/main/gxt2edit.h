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
private:
	enum eFileType
	{
		FILETYPE_UNKNOWN,

		FILETYPE_GXT2,
		FILETYPE_TXT,
		FILETYPE_JSON,

		FILETYPE_MAX
	};
public:
	gxt2edit(const string& windowTitle, int width, int height);
	virtual ~gxt2edit();
public:
	int Run(int argc, char* argv[]) override;
private:
	void Reset();

	//---------------- UI ----------------
	//
	void OnTick() override;
	void RenderBar();
	void RenderTable();
	void RenderEditor();
	void ProcessShortcuts();

	//---------------- IO ----------------
	//
	void OpenFile();
	void SaveFile();
	void SaveFileAs();
	void SaveToFile(const string& path, eFileType fileType);
	void LoadFromFile(const string& path, eFileType fileType);

	//---------------- Editing ----------------
	//
	void UpdateEntries();
	void FlagForDeletion(unsigned int uHash);

private:
	//---------------- IO ----------------
	//
	CFile::Map m_Data;
	string m_Path;

	//---------------- UI ----------------
	//
	ImVec2 m_BarSize;
	string m_HashInput;
	string m_LabelInput;
	string m_TextInput;

	//---------------- Editing ----------------
	//
	vector<unsigned int> m_EntriesToRemove;
};

#endif // _GXT2EDIT_H_