//
//	main/gxt2edit.h
//

#ifndef _GXT2EDIT_H_
#define _GXT2EDIT_H_

// Project
#include "gxt/gxt2.h"
#include "grc/image.h"
#include "system/app.h"

// vendor
#include <imgui.h>

// C/C++
#include <vector>
#include <stack>

// Descriptions
#define FILEDESC_GXT2 "GTA Text Table (*.gxt2)"
#define FILEDESC_JSON "JSON File (*.json)"
#define FILEDESC_CSV  "CSV File (*.csv)"
#define FILEDESC_OXT  "Open Office (*.oxt)"
#define FILEDESC_TEXT "Text File (*.txt)"
#define FILEDESC_ALL  "All Files (*.*)"
#define FILTERSPEC_GXT2 "*.gxt2"
#define FILTERSPEC_JSON "*.json"
#define FILTERSPEC_CSV  "*.csv"
#define FILTERSPEC_OXT  "*.oxt"
#define FILTERSPEC_TEXT "*.txt"
#define FILTERSPEC_ALL  "*.*"

// File Extension
#define FILE_EXTENSION_GXT2			TEXT(".gxt2")
#define FILE_EXTENSION_HANDLER		TEXT("GXT2TextFile")
#define FILE_EXTENSION_DESC			TEXT("GTA Text Table")

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
private:
	gxt2edit(const std::string& windowTitle, int width, int height);
	virtual ~gxt2edit();
public:
	int Run(int argc, char* argv[]) override;
public:
	static gxt2edit& GetInstance();
private:
	//---------------- App ----------------
	//
	bool Init() override;
	void Shutdown() override;
	void Reset();

#if _WIN32
	void RegisterExtension(bool bUnregister = false);
#endif

	//---------------- UI ----------------
	//
public:
	void Draw() override;
private:
	void RenderMenuBar();
	void RenderEditor();
	void RenderTable();
	void RenderEmptyView();
	void RenderEditTools();
	void RenderPopups();
	void ProcessShortcuts();
	void SortTable();

	//---------------- IO ----------------
	//
	void NewFile();
	void OpenFile();
	void CloseFile();
	void ImportFile();
	void ExportFile();
	void SaveFile();
	void SaveFileAs();
	void SaveToFile(const std::string& path, eFileType fileType);
	void LoadFromFile(const std::string& path, eFileType fileType);
	void ProcessFileRequests();
	void HandleDragDropLoading();
	void HandleWindowClosing();

	void SetEndian(int endian) { m_Endian = endian; }
	void SetLittleEndian() { m_Endian = CFile::_LITTLE_ENDIAN; }
	void SetBigEndian() { m_Endian = CFile::_BIG_ENDIAN; }
	bool IsLittleEndian() const { return m_Endian == CFile::_LITTLE_ENDIAN; }
	bool IsBigEndian() const { return m_Endian == CFile::_BIG_ENDIAN; }
	int GetEndian() const { return m_Endian; }

	//---------------- Editing ----------------
	//
	bool CheckChanges();
	void FlagForDeletion(unsigned int uHash);
	void UpdateFilter();
	void UpdateEntries();

private:
	//---------------- IO ----------------
	//
	CFile::Vec m_Data;
	CFile::Vec m_Filter;
	std::string m_Path;
	int m_Endian;

	//---------------- UI ----------------
	//
	ImVec2 m_BarSize;
	std::string m_HashInput;
	std::string m_LabelInput;
	std::string m_TextInput;
	std::string m_SearchInput;
	CImage* m_AddFileImg;

	//---------------- Editing ----------------
	//
	bool m_RequestNewFile;
	bool m_RequestOpenFile;
	bool m_RequestCloseFile;
	bool m_RequestImportFile;

	bool m_HasPendingChanges;
	bool m_RenderSaveChangesPopup;
	bool m_RenderEmptyEditorTable;
	bool m_RenderEntryAlreadyExistPopup;
	bool m_OverrideExistingEntry;
	std::vector<unsigned int> m_EntriesToRemove;
	std::stack<std::pair<unsigned int, std::string>> m_ItemsToAdd;
};

#endif // !_GXT2EDIT_H_