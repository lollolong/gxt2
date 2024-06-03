//
//	main/gxt2edit.cpp
//

// Project
#include "main.h"
#include "gxt2edit.h"
#include "data/util.h"
#include "data/stringhash.h"
#include "grc/graphics.h"
#include "grc/images/addfile.cpp"
#include "resources/resource.h"

// C/C++
#include <vector>
#include <format>
#include <algorithm>
#include <filesystem>

// vendor
#include <IconsFontAwesome6.h>

gxt2edit::gxt2edit(const std::string& windowTitle, int width, int height) :
	CAppUI(windowTitle, width, height),
	m_Endian(CFile::_LITTLE_ENDIAN),
	m_AddFileImg(nullptr),
	m_RequestNewFile(false),
	m_RequestOpenFile(false),
	m_RequestCloseFile(false),
	m_RequestImportFile(false),
	m_HasPendingChanges(false),
	m_RenderSaveChangesPopup(false),
	m_RenderEmptyEditorTable(true)
{
}

gxt2edit::~gxt2edit()
{
	Reset();
}

gxt2edit& gxt2edit::GetInstance()
{
	static gxt2edit gxt2edit("Text Table Editor", 1500, 850);
	return gxt2edit;
}

int gxt2edit::Run(int argc, char* argv[])
{
	if (argc == 2)
	{
		LoadFromFile(argv[1], FILETYPE_GXT2);
	}
	return CAppUI::Run(argc, argv);
}

bool gxt2edit::Init()
{
	const bool bInit = CAppUI::Init();
	m_AddFileImg = CImage::FromMemory(g_ImageAddFile);
	return bInit;
}

void gxt2edit::Shutdown()
{
	delete m_AddFileImg;
	return CAppUI::Shutdown();
}

void gxt2edit::Reset()
{
	m_Data.clear();
	m_Filter.clear();
}

void gxt2edit::Draw()
{
	HandleDragDropLoading();
	RenderPopups();
	RenderEditor();
	ProcessShortcuts();
	ProcessFileRequests();
	UpdateEntries();
	HandleWindowClosing();
}

void gxt2edit::RenderMenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem(ICON_FA_FILE "  New File", "CTRL + N"))
			{
				m_RequestNewFile = true;
			}
			if (ImGui::MenuItem(ICON_FA_FOLDER_OPEN " Open File", "CTRL + O"))
			{
				m_RequestOpenFile = true;
			}
			if (ImGui::MenuItem(ICON_FA_FOLDER_CLOSED " Close File", "CTRL + W", false, !m_RenderEmptyEditorTable))
			{
				m_RequestCloseFile = true;
			}
			ImGui::Separator();
			if (ImGui::MenuItem(ICON_FA_FLOPPY_DISK "  Save", "CTRL + S", false, !m_Data.empty()))
			{
				SaveFile();
			}
			if (ImGui::MenuItem(ICON_FA_COPY "  Save As", "CTRL + SHIFT + S", false, !m_Data.empty()))
			{
				SaveFileAs();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Import and Export"))
		{
			if (ImGui::MenuItem(ICON_FA_FILE_IMPORT " Import", "CTRL + I"))
			{
				m_RequestImportFile = true;
			}
			if (ImGui::MenuItem(ICON_FA_FILE_EXPORT " Export", "CTRL + E", false, !m_Data.empty()))
			{
				ExportFile();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Settings"))
		{
			if (ImGui::MenuItem("Little Endian", nullptr, IsLittleEndian()))
			{
				SetLittleEndian();
			}
			if (ImGui::MenuItem("Big Endian", nullptr, IsBigEndian()))
			{
				SetBigEndian();
			}
#if _WIN32
			ImGui::Separator();
			if (ImGui::MenuItem("Register Extension", "Requires Admin"))
			{
				RegisterExtension();
			}
			if (ImGui::MenuItem("Unregister Extension", "Requires Admin"))
			{
				RegisterExtension(true);
			}
#endif
			ImGui::Separator();
			if (ImGui::MenuItem("Exit", "ALT + F4"))
			{
				CGraphics::SetIsClosing(true);
			}
			ImGui::EndMenu();
		}

		m_BarSize = ImGui::GetWindowSize();
		ImGui::EndMainMenuBar();
	}
}

void gxt2edit::RenderEditor()
{
	RenderMenuBar();
	if (!m_Data.empty() || !m_RenderEmptyEditorTable)
	{
		RenderTable();
		RenderEditTools();
	}
	else
	{
		RenderEmptyView();
	}
}

void gxt2edit::RenderTable()
{
	const ImGuiViewport* pViewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(ImVec2(pViewport->Pos.x, m_BarSize.y));
	ImGui::SetNextWindowSize(ImVec2(pViewport->Size.x, pViewport->Size.y - m_BarSize.y - 130.f));

	if (ImGui::Begin("##Editor", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
	{
		if (ImGui::BeginTable("GXT2 Editor", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable))
		{
			ImGui::TableSetupColumn("", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoResize | ImGuiTabItemFlags_NoReorder | ImGuiTableColumnFlags_NoSort);
			ImGui::TableSetupColumn("Hash", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoResize);
			ImGui::TableSetupColumn("Text", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupScrollFreeze(0, 1);
			ImGui::TableHeadersRow();

			SortTable();

			ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.f, 0.f));
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, ImGui::GetStyle().FramePadding.y));

			if (!m_Filter.empty())
			{
				ImGuiListClipper clipper;
				clipper.Begin(static_cast<int>(m_Filter.size()));

				const float trashIconWidth = ImGui::CalcTextSize(ICON_FA_TRASH).x;

				while (clipper.Step())
				{
					for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
					{
						const unsigned int& uHash = m_Filter[i].first;
						std::string& text = m_Filter[i].second;

						std::string szHash = std::format("0x{:08X}", uHash);

						ImGui::TableNextRow();

						// Delete column
						ImGui::TableSetColumnIndex(0);
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - trashIconWidth) * 0.5f);
						if (ImGui::Button((ICON_FA_TRASH "##" + std::to_string(uHash)).c_str()))
						{
							FlagForDeletion(uHash);
						}
						ImGui::PopStyleColor();

						// Hash column
						ImGui::TableSetColumnIndex(1);
						ImGui::PushItemWidth(90.f);
						ImGui::InputText(("##Hash" + szHash).c_str(), &szHash, ImGuiInputTextFlags_ReadOnly);
						ImGui::PopItemWidth();

						// Text column
						ImGui::TableSetColumnIndex(2);
						ImGui::PushItemWidth(-FLT_EPSILON);
						if (ImGui::InputText(("##Text" + std::to_string(uHash)).c_str(), &text, ImGuiInputTextFlags_AutoSelectAll))
						{
							if (text.empty())
							{
								FlagForDeletion(uHash);
							}
							m_HasPendingChanges = true;
						}
						ImGui::PopItemWidth();
					}
				}
			}

			ImGui::PopStyleColor();
			ImGui::PopStyleVar();
			ImGui::PopStyleVar();

			ImGui::EndTable();
		}
		ImGui::End();
	}
}

void gxt2edit::RenderEmptyView()
{
	const ImGuiViewport* pViewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(ImVec2(pViewport->Pos.x, m_BarSize.y));
	ImGui::SetNextWindowSize(ImVec2(pViewport->Size.x, pViewport->Size.y - m_BarSize.y));

	if (ImGui::Begin("##EmptyEditor", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
	{
		const ImVec2 windowSize = ImGui::GetWindowSize();

		const float imageWidth = static_cast<float>(m_AddFileImg->GetWidth());
		const float imageHeight = static_cast<float>(m_AddFileImg->GetHeight());

		const float imageSizeY = windowSize.y * 0.20f;
		const float imageSizeX = imageSizeY * (imageWidth / imageHeight);

		const float posX = (windowSize.x - imageSizeX) / 2.0f;
		const float posY = (windowSize.y - imageSizeY) / 2.0f;

		ImGui::SetCursorPos(ImVec2(posX, posY));
		ImGui::Image(m_AddFileImg->GetTextureId(), ImVec2(imageSizeX, imageSizeY));

		ImGui::End();
	}
}

void gxt2edit::RenderEditTools()
{
	const ImGuiViewport* pViewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(ImVec2(pViewport->Pos.x, pViewport->Size.y - 130.f));
	ImGui::SetNextWindowSize(ImVec2(pViewport->Size.x, 130.f));

	if (ImGui::Begin("Editor Bar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
	{
		ImGui::AlignTextToFramePadding();
		ImGui::Text(ICON_FA_CODE " Hash");
		ImGui::SameLine();

		ImGui::PushItemWidth(90.f);
		ImGui::InputText("##HashInput", &m_HashInput, ImGuiInputTextFlags_CharsHexadecimal);
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::Text(ICON_FA_TAGS " Label");
		ImGui::SameLine();

		ImGui::PushItemWidth(250.f);
		if (ImGui::InputText("##LabelInput", &m_LabelInput))
		{
			m_HashInput = std::format("{:08X}", rage::atStringHash(m_LabelInput.c_str()));
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::Text(ICON_FA_PENCIL " Text");
		ImGui::SameLine();

		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 50.f);
		ImGui::InputText("##TextInput", &m_TextInput);
		ImGui::PopItemWidth();
		ImGui::SameLine();

		if (ImGui::Button("Add"))
		{
			if (!m_TextInput.empty())
			{
				const unsigned int uHash = strtoul(m_HashInput.c_str(), nullptr, 16);

				if (uHash != 0x00000000)
				{
					m_Data.emplace_back(uHash, m_TextInput);
					UpdateFilter();

					m_HashInput.clear();
					m_LabelInput.clear();
					m_TextInput.clear();

					m_HasPendingChanges = true;
				}
			}
		}

		ImGui::NewLine();

		ImGui::AlignTextToFramePadding();
		ImGui::Text(ICON_FA_MAGNIFYING_GLASS " Search");
		ImGui::SameLine();

		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
		if (ImGui::InputText("##SearchInput", &m_SearchInput))
		{
			UpdateFilter();
		}
		ImGui::PopItemWidth();

		ImGui::End();
	}
}

void gxt2edit::RenderPopups()
{
	if (ImGui::BeginPopupModal("Unsaved Changes", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("You have unsaved changes. Do you want to save them now?");
		ImGui::Separator();

		if (ImGui::Button("Save", ImVec2(120, 0)))
		{
			SaveFile();
			m_RenderSaveChangesPopup = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Don't Save", ImVec2(120, 0)))
		{
			m_RenderSaveChangesPopup = false;
			m_HasPendingChanges = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			m_RequestNewFile = false;
			m_RequestOpenFile = false;
			m_RequestImportFile = false;
			m_RenderSaveChangesPopup = false;
			CGraphics::SetIsClosing(false);
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	if (m_RenderSaveChangesPopup)
	{
		ImGui::OpenPopup("Unsaved Changes");
	}
}

void gxt2edit::ProcessShortcuts()
{
	ImGuiIO& io = ImGui::GetIO();

	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_N)))
	{
		m_RequestNewFile = true;
	}
	else if (io.KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_O)))
	{
		m_RequestOpenFile = true;
	}
	else if (io.KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_W)))
	{
		m_RequestCloseFile = true;
	}
	else if (io.KeyCtrl && io.KeyShift && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_S)) && !m_Data.empty())
	{
		SaveFileAs();
	}
	else if (io.KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_S)) && !m_Data.empty())
	{
		SaveFile();
	}
	else if (io.KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_I)))
	{
		m_RequestImportFile = true;
	}
	else if (io.KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_E)) && !m_Data.empty())
	{
		ExportFile();
	}
}

void gxt2edit::SortTable()
{
	ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs();
	if (sortSpecs && sortSpecs->SpecsDirty)
	{
		auto compareEntries = [&](const std::pair<unsigned int, std::string>& a, const std::pair<unsigned int, std::string>& b) -> bool
		{
			for (int n = 0; n < sortSpecs->SpecsCount; n++)
			{
				const ImGuiTableColumnSortSpecs* sortSpec = &sortSpecs->Specs[n];
				int delta = 0;
				switch (sortSpec->ColumnIndex)
				{
				case 1:
					if (a.first < b.first) delta = -1;
					if (a.first > b.first) delta = 1;
					break;
				case 2:
					delta = a.second.compare(b.second);
					break;
				}
				if (delta != 0)
					return (sortSpec->SortDirection == ImGuiSortDirection_Ascending) ? (delta < 0) : (delta > 0);
			}
			return false;
		};

		std::sort(m_Filter.begin(), m_Filter.end(), compareEntries);
		sortSpecs->SpecsDirty = false;
	}
}

void gxt2edit::NewFile()
{
	if (!CheckChanges())
	{
		return;
	}
	Reset();
	m_Path.clear();
	m_RequestNewFile = false;
	m_RenderEmptyEditorTable = false;
}

void gxt2edit::OpenFile()
{
	if (!CheckChanges())
	{
		return;
	}

	if (utils::OpenFileExplorerDialog("Select a GTA Text Table", "", m_Path, false, {FILEDESC_GXT2, FILTERSPEC_GXT2}))
	{
		Reset();
		LoadFromFile(m_Path, FILETYPE_GXT2);
	}

	m_RequestOpenFile = false;
}

void gxt2edit::CloseFile()
{
	if (!CheckChanges())
	{
		return;
	}
	Reset();
	m_Path.clear();
	m_RequestCloseFile = false;
	m_RenderEmptyEditorTable = true;
}

void gxt2edit::ImportFile()
{
	if (!CheckChanges())
	{
		return;
	}

	const std::string backupPath = m_Path;

	if (utils::OpenFileExplorerDialog("Import Text Table (JSON, CSV, OXT, TXT)", "", m_Path, false,
		{
			FILEDESC_ALL, FILTERSPEC_ALL,
			FILEDESC_JSON, FILTERSPEC_JSON,
			FILEDESC_CSV, FILTERSPEC_CSV,
			FILEDESC_OXT, FILTERSPEC_OXT,
			FILEDESC_TEXT, FILTERSPEC_TEXT,
		}
		))
	{
		eFileType fileType = FILETYPE_UNKNOWN;
		const std::string szInputExtension = m_Path.substr(m_Path.find_last_of("."));

		if (szInputExtension == ".gxt2")
		{
			fileType = FILETYPE_GXT2;
		}
		else if (szInputExtension == ".txt")
		{
			fileType = FILETYPE_TXT;
		}
		else if (szInputExtension == ".json")
		{
			fileType = FILETYPE_JSON;
		}
		else if (szInputExtension == ".csv")
		{
			fileType = FILETYPE_CSV;
		}
		else if (szInputExtension == ".oxt")
		{
			fileType = FILETYPE_OXT;
		}

		if (fileType != FILETYPE_UNKNOWN)
		{
			Reset();
			LoadFromFile(m_Path, fileType);
			m_Path = backupPath;
		}
	}

	m_RequestImportFile = false;
}

void gxt2edit::ExportFile()
{
	const std::string backupPath = m_Path;

	if (utils::OpenFileExplorerDialog("Export Text Table (JSON, CSV, OXT, TXT)", "export.json", m_Path, true,
		{
			FILEDESC_ALL, FILTERSPEC_ALL ,
			FILEDESC_JSON, FILTERSPEC_JSON ,
			FILEDESC_CSV, FILTERSPEC_CSV ,
			FILEDESC_OXT, FILTERSPEC_OXT ,
			FILEDESC_TEXT, FILTERSPEC_TEXT ,
		}
		))
	{
		eFileType fileType = FILETYPE_GXT2;

		const size_t n = m_Path.find_last_of(".");
		if (n != std::string::npos)
		{
			const std::string szInputExtension = m_Path.substr(n);

			if (szInputExtension == ".gxt2")
			{
				fileType = FILETYPE_GXT2;
			}
			else if (szInputExtension == ".txt")
			{
				fileType = FILETYPE_TXT;
			}
			else if (szInputExtension == ".json")
			{
				fileType = FILETYPE_JSON;
			}
			else if (szInputExtension == ".csv")
			{
				fileType = FILETYPE_CSV;
			}
			else if (szInputExtension == ".oxt")
			{
				fileType = FILETYPE_OXT;
			}
		}

		if (fileType != FILETYPE_UNKNOWN)
		{
			SaveToFile(m_Path, fileType);
			m_Path = backupPath;
		}
	}
}

void gxt2edit::SaveFile()
{
	if (!m_Path.empty())
	{
		SaveToFile(m_Path, FILETYPE_GXT2);
	}
	else
	{
		SaveFileAs();
	}
}

void gxt2edit::SaveFileAs()
{
	if (utils::OpenFileExplorerDialog("Save GTA Text Table", "textfile.gxt2", m_Path, true, { FILEDESC_GXT2, FILTERSPEC_GXT2 }))
	{
		SaveToFile(m_Path, FILETYPE_GXT2);
	}
}

void gxt2edit::SaveToFile(const std::string& path, eFileType fileType)
{
	CFile* pOutputDevice = nullptr;

	switch (fileType)
	{
	case FILETYPE_GXT2:
		pOutputDevice = GXT_NEW CGxt2File(path, CFile::FLAGS_WRITE_COMPILED, GetEndian());
		break;
	case FILETYPE_TXT:
		pOutputDevice = GXT_NEW CTextFile(path, CFile::FLAGS_WRITE_DECOMPILED);
		break;
	case FILETYPE_JSON:
		pOutputDevice = GXT_NEW CJsonFile(path, CFile::FLAGS_WRITE_DECOMPILED);
		break;
	case FILETYPE_CSV:
		pOutputDevice = GXT_NEW CCsvFile(path, CFile::FLAGS_WRITE_DECOMPILED);
		break;
	case FILETYPE_OXT:
		pOutputDevice = GXT_NEW COxtFile(path, CFile::FLAGS_WRITE_DECOMPILED);
		break;
	default:
		break;
	}

	if (pOutputDevice)
	{
		pOutputDevice->SetData(m_Data);
		if (pOutputDevice->WriteEntries() && fileType == FILETYPE_GXT2)
		{
			m_Path = path;
		}
		m_HasPendingChanges = false;
		delete pOutputDevice;
	}
}

void gxt2edit::LoadFromFile(const std::string& path, eFileType fileType)
{
	CFile* pInputDevice = nullptr;

	switch (fileType)
	{
	case FILETYPE_GXT2:
		pInputDevice = GXT_NEW CGxt2File(path, CFile::FLAGS_READ_COMPILED);
		break;
	case FILETYPE_TXT:
		pInputDevice = GXT_NEW CTextFile(path, CFile::FLAGS_READ_DECOMPILED);
		break;
	case FILETYPE_JSON:
		pInputDevice = GXT_NEW CJsonFile(path, CFile::FLAGS_READ_DECOMPILED);
		break;
	case FILETYPE_CSV:
		pInputDevice = GXT_NEW CCsvFile(path, CFile::FLAGS_READ_DECOMPILED);
		break;
	case FILETYPE_OXT:
		pInputDevice = GXT_NEW COxtFile(path, CFile::FLAGS_READ_DECOMPILED);
		break;
	default:
		break;
	}

	if (pInputDevice)
	{
		pInputDevice->SetData(m_Data);
		if (pInputDevice->ReadEntries())
		{
			for (const auto& [uHash, szEntry] : pInputDevice->GetData())
			{
				m_Data.emplace_back(uHash, szEntry);
			}
			m_Filter = m_Data;

			if (fileType == FILETYPE_GXT2)
			{
				SetEndian(pInputDevice->GetEndian());
				m_Path = path;
			}
			m_RenderEmptyEditorTable = false;
		}
		delete pInputDevice;
	}
}

void gxt2edit::ProcessFileRequests()
{
	if (m_RequestNewFile)
	{
		NewFile();
	}
	if (m_RequestOpenFile)
	{
		OpenFile();
	}
	if (m_RequestCloseFile)
	{
		CloseFile();
	}
	if (m_RequestImportFile)
	{
		ImportFile();
	}
}

void gxt2edit::HandleDragDropLoading()
{
	if (!CGraphics::GetDropFiles().empty())
	{
		const std::string dropPath = CGraphics::GetDropFiles().top();
		CGraphics::GetDropFiles().pop();

		if (CheckChanges() && std::filesystem::exists(dropPath) && std::filesystem::path(dropPath).extension() == ".gxt2")
		{
			Reset();
			LoadFromFile(dropPath, FILETYPE_GXT2);
		}
	}
}

void gxt2edit::HandleWindowClosing()
{
	if (CGraphics::IsClosing())
	{
		CGraphics::GetInstance().CloseWindow(CheckChanges());
	}
}

void gxt2edit::FlagForDeletion(unsigned int uHash)
{
	m_HasPendingChanges = true;
	m_EntriesToRemove.push_back(uHash);
}

void gxt2edit::UpdateFilter()
{
	m_Filter.clear();

	if (m_Data.empty())
	{
		m_HasPendingChanges = false;
		return;
	}
	if (m_SearchInput.empty())
	{
		m_Filter = m_Data;
		return;
	}
	for (const auto& entry : m_Data)
	{
		if (entry.second.find(m_SearchInput) != std::string::npos)
		{
			m_Filter.push_back(entry);
		}
	}
}

bool gxt2edit::CheckChanges()
{
	if (m_HasPendingChanges)
	{
		m_RenderSaveChangesPopup = true;
		return false;
	}
	return true;
}

void gxt2edit::UpdateEntries()
{
	for (const unsigned int& uHash : m_EntriesToRemove)
	{
		auto it = std::find_if(m_Data.begin(), m_Data.end(),
			[uHash](const std::pair<unsigned int, std::string>& entry) -> bool
			{
				return entry.first == uHash;
			});

		if (it != m_Data.end())
		{
			m_Data.erase(it);
		}
	}
	UpdateFilter();
	m_EntriesToRemove.clear();
}

#if _WIN32
void gxt2edit::RegisterExtension(bool bUnregister /*= false*/)
{
	const bool bRefreshShell = 
		bUnregister ? 
			SUCCEEDED(utils::UnregisterShellFileExtension(FILE_EXTENSION_GXT2, FILE_EXTENSION_HANDLER)) : 
			SUCCEEDED(utils::RegisterShellFileExtension(FILE_EXTENSION_GXT2, FILE_EXTENSION_HANDLER, FILE_EXTENSION_DESC));

	if (bRefreshShell)
	{
		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	}
}
#endif


//---------------- Entry Point ----------------
//

#ifdef _WIN32
	//#define WINMAIN_ENTRY_DEBUG
	#if defined(_DEBUG) && !defined(WINMAIN_ENTRY_DEBUG)
		#pragma comment(linker, "/SUBSYSTEM:CONSOLE")
	#else
		#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
	#endif
#endif

#if (defined(_DEBUG) && !defined(WINMAIN_ENTRY_DEBUG)) || !defined(_WIN32)
int main(int argc, char* argv[])
{
#elif _WIN32
int WINAPI WinMain(
	[[maybe_unused]] _In_ HINSTANCE			hInstance,
	[[maybe_unused]] _In_opt_ HINSTANCE		hPrevInstance,
	[[maybe_unused]] _In_ LPSTR				lpCmdLine,
	[[maybe_unused]] _In_ int				nCmdShow
)
{
#endif
	try
	{
#if defined(_WIN32) && (!defined(_DEBUG) || defined(WINMAIN_ENTRY_DEBUG))
		int argc = 0;
		char** argv = nullptr;

		LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
		if (argvW)
		{
			argv = (char**)malloc(sizeof(char*) * argc);
			if (argv)
			{
				for (int i = 0; i < argc; i++)
				{
					argv[i] = nullptr;
				}

				for (int j = 0; j < argc; j++)
				{
					size_t wcharLen = wcslen(argvW[j]);
					argv[j] = (char*)malloc(sizeof(char) * (wcharLen + 1));
					if (argv[j])
					{
						size_t numConverted = 0;
						wcstombs_s(&numConverted, argv[j], wcharLen + 1, argvW[j], wcharLen);
					}
					else
					{
						for (int i = 0; i < argc; i++)
						{
							free(argv[i]);
							argv[i] = nullptr;
						}
						free(argv);
						argv = nullptr;
						argc = 0;
						break;
					}
				}
			}
			LocalFree(argvW);
		}
#endif

		//---------------- Main Logic ----------------
		//
		gxt2edit::GetInstance().Run(argc, argv);

#if defined(_WIN32) && (!defined(_DEBUG) || defined(WINMAIN_ENTRY_DEBUG))
		if (argc && argv)
		{
			for (int i = 0; i < argc; i++)
			{
				if (argv[i])
				{
					free(argv[i]);
					argv[i] = nullptr;
				}
			}
			free(argv);
			argv = nullptr;
		}
#endif
	}
	catch (const std::exception& ex)
	{
		printf("Error: %s\n", ex.what());
		return 1;
	}
	catch (...)
	{
		printf("Unknown error occurred!\n");
		return 1;
	}
	return 0;
}