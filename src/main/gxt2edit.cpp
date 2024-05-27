//
//	main/gxt2edit.cpp
//

// Windows
#include <Windows.h>

// Project
#include "gxt2edit.h"
#include "data/util.h"
#include "data/stringhash.h"
#include "grc/graphics.h"
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
	m_HasPendingChanges(false),
	m_RenderSaveChangesPopup(false),
	m_RequestNewFile(false),
	m_RequestOpenFile(false),
	m_RequestImportFile(false)
{
}

gxt2edit::~gxt2edit()
{
	Reset();
}

int gxt2edit::Run(int argc, char* argv[])
{
	if (argc == 2)
	{
		LoadFromFile(argv[1], FILETYPE_GXT2);
	}
	return CAppUI::Run(argc, argv);
}

void gxt2edit::Reset()
{
	m_Data.clear();
}

void gxt2edit::OnTick()
{
#if !_DEBUG
	ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif

	HandleDragDropLoading();
	RenderPopups();
	RenderBar();
	RenderTable();
	RenderEditor();
	ProcessShortcuts();
	ProcessFileRequests();
	UpdateEntries();
}

void gxt2edit::RenderBar()
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
			if (ImGui::MenuItem("Register Extension", "Requires Admin"))
			{
				if (SUCCEEDED(utils::RegisterShellFileExtension(TEXT(".gxt2"), TEXT("GXT2TextFile"), TEXT("GTA Text Table"))))
				{
					SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
				}
			}
			if (ImGui::MenuItem("Unregister Extension", "Requires Admin"))
			{
				if (SUCCEEDED(utils::UnregisterShellFileExtension(TEXT(".gxt2"), TEXT("GXT2TextFile"))))
				{
					SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
				}
				
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Exit", "ALT + F4"))
			{
				exit(0);
			}
			ImGui::EndMenu();
		}

		m_BarSize = ImGui::GetWindowSize();
		ImGui::EndMainMenuBar();
	}
}

void gxt2edit::RenderTable()
{
	const ImGuiViewport* pViewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(ImVec2(pViewport->Pos.x, m_BarSize.y));
	ImGui::SetNextWindowSize(ImVec2(pViewport->Size.x, pViewport->Size.y - m_BarSize.y - 65.f));

	if (ImGui::Begin("##Editor", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
	{
		if (ImGui::BeginTable("GXT2 Editor", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable))
		{
			ImGui::TableSetupColumn("", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoResize  | ImGuiTabItemFlags_NoReorder | ImGuiTableColumnFlags_NoSort);
			ImGui::TableSetupColumn("Hash", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoResize);
			ImGui::TableSetupColumn("Text", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupScrollFreeze(0, 1);
			ImGui::TableHeadersRow();

			SortTable();

			ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.f, 0.f));
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, ImGui::GetStyle().FramePadding.y));

			if (!m_Data.empty())
			{
				ImGuiListClipper clipper;
				clipper.Begin(static_cast<int>(m_Data.size()));

				const float trashIconWidth = ImGui::CalcTextSize(ICON_FA_TRASH).x;

				while (clipper.Step())
				{
					for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
					{
						const unsigned int& uHash = m_Data[i].first;
						std::string& text = m_Data[i].second;

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

void gxt2edit::RenderEditor()
{
	const ImGuiViewport* pViewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(ImVec2(pViewport->Pos.x, pViewport->Size.y - 65.f));
	ImGui::SetNextWindowSize(ImVec2(pViewport->Size.x, 65.f));

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

					m_HashInput.clear();
					m_LabelInput.clear();
					m_TextInput.clear();

					m_HasPendingChanges = true;
				}
			}
		}

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
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_O)))
	{
		m_RequestOpenFile = true;
	}
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_S)) && !m_Data.empty())
	{
		SaveFile();
	}
	if (io.KeyCtrl && io.KeyShift && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_S)) && !m_Data.empty())
	{
		SaveFileAs();
	}
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_I)))
	{
		m_RequestImportFile = true;
	}
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_E)) && !m_Data.empty())
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

		std::sort(m_Data.begin(), m_Data.end(), compareEntries);
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
}

void gxt2edit::OpenFile()
{
	if (!CheckChanges())
	{
		return;
	}

	if (utils::OpenFileExplorerDialog(L"Select a GTA Text Table", L"", m_Path, false, {{FILEDESC_GXT2, FILTERSPEC_GXT2}}))
	{
		Reset();
		LoadFromFile(m_Path, FILETYPE_GXT2);
	}
	m_RequestOpenFile = false;
}

void gxt2edit::ImportFile()
{
	if (!CheckChanges())
	{
		return;
	}

	const std::string backupPath = m_Path;
	if (utils::OpenFileExplorerDialog(L"Import Text Table (JSON, CSV, OXT, TXT)", L"", m_Path, false,
		{
			{ FILEDESC_ALL, FILTERSPEC_ALL },
			{ FILEDESC_JSON, FILTERSPEC_JSON },
			{ FILEDESC_CSV, FILTERSPEC_CSV },
			{ FILEDESC_OXT, FILTERSPEC_OXT },
			{ FILEDESC_TEXT, FILTERSPEC_TEXT },
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
	if (utils::OpenFileExplorerDialog(L"Export Text Table (JSON, CSV, OXT, TXT)", L".json", m_Path, true,
		{
			{ FILEDESC_ALL, FILTERSPEC_ALL },
			{ FILEDESC_JSON, FILTERSPEC_JSON },
			{ FILEDESC_CSV, FILTERSPEC_CSV },
			{ FILEDESC_OXT, FILTERSPEC_OXT },
			{ FILEDESC_TEXT, FILTERSPEC_TEXT },
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
	if (utils::OpenFileExplorerDialog(L"Save GTA Text Table", L"textfile.gxt2", m_Path, true, { { FILEDESC_GXT2, FILTERSPEC_GXT2 } }))
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
		pOutputDevice = new CGxt2File(path, CFile::FLAGS_WRITE_COMPILED);
		break;
	case FILETYPE_TXT:
		pOutputDevice = new CTextFile(path, CFile::FLAGS_WRITE_DECOMPILED);
		break;
	case FILETYPE_JSON:
		pOutputDevice = new CJsonFile(path, CFile::FLAGS_WRITE_DECOMPILED);
		break;
	case FILETYPE_CSV:
		pOutputDevice = new CCsvFile(path, CFile::FLAGS_WRITE_DECOMPILED);
		break;
	case FILETYPE_OXT:
		pOutputDevice = new COxtFile(path, CFile::FLAGS_WRITE_DECOMPILED);
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
		pInputDevice = new CGxt2File(path, CFile::FLAGS_READ_COMPILED);
		break;
	case FILETYPE_TXT:
		pInputDevice = new CTextFile(path, CFile::FLAGS_READ_DECOMPILED);
		break;
	case FILETYPE_JSON:
		pInputDevice = new CJsonFile(path, CFile::FLAGS_READ_DECOMPILED);
		break;
	case FILETYPE_CSV:
		pInputDevice = new CCsvFile(path, CFile::FLAGS_READ_DECOMPILED);
		break;
	case FILETYPE_OXT:
		pInputDevice = new COxtFile(path, CFile::FLAGS_READ_DECOMPILED);
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
			if (fileType == FILETYPE_GXT2)
			{
				m_Path = path;
			}
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
	if (m_RequestImportFile)
	{
		ImportFile();
	}
}

void gxt2edit::HandleDragDropLoading()
{
	if (!CGraphics::sm_DropFiles.empty())
	{
		const std::string dropPath = CGraphics::sm_DropFiles.top();
		CGraphics::sm_DropFiles.pop();

		if (CheckChanges() && std::filesystem::exists(dropPath) && std::filesystem::path(dropPath).extension() == ".gxt2")
		{
			Reset();
			LoadFromFile(dropPath, FILETYPE_GXT2);
		}
	}
}

void gxt2edit::FlagForDeletion(unsigned int uHash)
{
	m_HasPendingChanges = true;
	m_EntriesToRemove.push_back(uHash);
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
			[uHash](const std::pair<unsigned int, std::string>& entry) 
			{
				return entry.first == uHash;
			});

		if (it != m_Data.end())
		{
			m_Data.erase(it);
		}
	}
	m_EntriesToRemove.clear();
}

int main(int argc, char* argv[])
{
#if _DEBUG
	ShowWindow(GetConsoleWindow(), SW_SHOW);
#else
	ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif

	try
	{
		gxt2edit gxt2edit("Grand Theft Auto V - Text Editor", 1500, 850);
		gxt2edit.Run(argc, argv);
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