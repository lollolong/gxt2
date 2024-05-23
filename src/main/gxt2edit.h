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

// Descriptions
#define FILEDESC_GXT2 L"GTA Text Table (*.gxt2)"
#define FILEDESC_JSON L"JSON File (*.json)"
#define FILEDESC_CSV  L"CSV File (*.csv)"
#define FILEDESC_TEXT L"Text File (*.txt)"
#define FILTERSPEC_GXT2 L"*.gxt2"
#define FILTERSPEC_JSON L"*.json"
#define FILTERSPEC_CSV  L"*.csv"
#define FILTERSPEC_TEXT L"*.txt"

class gxt2edit : public CAppUI
{
private:
	enum eFileType
	{
		FILETYPE_UNKNOWN,

		FILETYPE_GXT2,
		FILETYPE_TXT,
		FILETYPE_JSON,
		FILETYPE_CSV,

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
	void RenderPopups();
	void ProcessShortcuts();

	//---------------- IO ----------------
	//
	void NewFile();
	void OpenFile();
	void ImportFile();
	void ExportFile();
	void SaveFile(bool checkForChanges = true);
	void SaveFileAs(bool checkForChanges = true);
	void SaveToFile(const string& path, eFileType fileType);
	void LoadFromFile(const string& path, eFileType fileType);
	void ProcessFileRequests();

	//---------------- Editing ----------------
	//
	bool CheckChanges();
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
	bool m_HasPendingChanges : 1;
	bool m_RenderSaveChangesPopup : 1;
	bool m_RequestNewFile : 1;
	bool m_RequestOpenFile : 1;
	bool m_RequestImportFile : 1;
	vector<unsigned int> m_EntriesToRemove;
};

#endif // _GXT2EDIT_H_