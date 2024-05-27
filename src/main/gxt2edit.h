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
#define FILEDESC_OXT  L"Open Office (*.oxt)"
#define FILEDESC_TEXT L"Text File (*.txt)"
#define FILEDESC_ALL  L"All Files (*.*)"
#define FILTERSPEC_GXT2 L"*.gxt2"
#define FILTERSPEC_JSON L"*.json"
#define FILTERSPEC_CSV  L"*.csv"
#define FILTERSPEC_OXT  L"*.oxt"
#define FILTERSPEC_TEXT L"*.txt"
#define FILTERSPEC_ALL  L"*.*"

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
		FILETYPE_OXT,

		FILETYPE_MAX
	};
public:
	gxt2edit(const std::string& windowTitle, int width, int height);
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
	void SortTable();

	//---------------- IO ----------------
	//
	void NewFile();
	void OpenFile();
	void ImportFile();
	void ExportFile();
	void SaveFile();
	void SaveFileAs();
	void SaveToFile(const std::string& path, eFileType fileType);
	void LoadFromFile(const std::string& path, eFileType fileType);
	void ProcessFileRequests();

	//---------------- Editing ----------------
	//
	bool CheckChanges();
	void UpdateEntries();
	void FlagForDeletion(unsigned int uHash);

private:
	//---------------- IO ----------------
	//
	CFile::Vec m_Data;
	std::string m_Path;

	//---------------- UI ----------------
	//
	ImVec2 m_BarSize;
	std::string m_HashInput;
	std::string m_LabelInput;
	std::string m_TextInput;

	//---------------- Editing ----------------
	//
	bool m_HasPendingChanges : 1;
	bool m_RenderSaveChangesPopup : 1;
	bool m_RequestNewFile : 1;
	bool m_RequestOpenFile : 1;
	bool m_RequestImportFile : 1;
	std::vector<unsigned int> m_EntriesToRemove;
};

#endif // _GXT2EDIT_H_