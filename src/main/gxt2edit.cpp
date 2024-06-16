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
#include <execution>

// vendor
#include <IconsFontAwesome6.h>

gxt2edit::gxt2edit(const std::string& windowTitle, int width, int height) :
	CAppUI(windowTitle, width, height),
	m_LabelNames(nullptr),
	m_Endian(CFile::_LITTLE_ENDIAN),
	m_LabelsNotFound(false),
	m_EditorToolsHeight(110.f),
	m_SortViewNextRound(false),
	m_SortUnderlyingData(false),
	m_AddFileImg(nullptr),
	m_RequestNewFile(false),
	m_RequestOpenFile(false),
	m_RequestCloseFile(false),
	m_RequestImportFile(false),
	m_HasPendingChanges(false),
	m_RenderSaveChangesPopup(false),
	m_RenderEmptyEditorTable(true),
	m_RenderEntryAlreadyExistPopup(false),
	m_OverrideExistingEntry(false),
	m_RenderEmptyHashPopup(false),
	m_RenderEmptyTextPopup(false)
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
	//---------------- System Init ----------------
	//
	const bool bInit = CAppUI::Init();

	//---------------- App Specific ----------------
	//
	m_AddFileImg = CImage::FromMemory(g_ImageAddFile);

#if _WIN32
	char szModulePath[MAX_PATH] = {};
	GetModuleFileNameA(NULL, szModulePath, sizeof(szModulePath));

	std::string modulePath = szModulePath;
	modulePath = modulePath.substr(0, modulePath.find_last_of('\\'));

	std::filesystem::path basePath = modulePath;
#else
	std::filesystem::path basePath = std::filesystem::current_path();

#if IS_APPIMAGE_BUILD
	basePath = std::getenv("APPDIR");
#endif

#endif

	std::filesystem::path labelsFile = basePath / LABELS_FILENAME;
	if (std::filesystem::exists(labelsFile))
	{
		m_LabelNames = GXT_NEW CHashDatabase(labelsFile.string());
		m_LabelNames->ReadEntries();
		m_LabelNames->Close();
	}
	else
	{
#if _WIN32
		TCHAR documentsPath[MAX_PATH] = {};
		if (SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS, NULL, 0, documentsPath) == S_OK)
		{
			basePath = documentsPath;
			basePath /= GXT_EDITOR_DOCUMENTS_PATH;

			labelsFile = basePath / LABELS_FILENAME;
		}
		if (std::filesystem::exists(labelsFile))
		{
			m_LabelNames = GXT_NEW CHashDatabase(labelsFile.string());
			m_LabelNames->ReadEntries();
			m_LabelNames->Close();
		}
		else
#endif
		{
			m_LabelNames = GXT_NEW CMemoryFile(); // memory device
			m_LabelsNotFound = true;
		}
	}

	return bInit;
}

void gxt2edit::Shutdown()
{
	if (m_AddFileImg)
	{
		delete m_AddFileImg;
		m_AddFileImg = nullptr;
	}
	if (m_LabelNames)
	{
		delete m_LabelNames;
		m_LabelNames = nullptr;
	}
	return CAppUI::Shutdown();
}

void gxt2edit::Reset()
{
	m_Data.clear();
	m_Filter.clear();

	m_HashInput.clear();
	m_LabelInput.clear();
	m_TextInput.clear();
}

void gxt2edit::Draw()
{
	HandleDragDropLoading();
	RenderPopups();
	CacheDisplayNames();
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
	ImGui::SetNextWindowSize(ImVec2(pViewport->Size.x, pViewport->Size.y - m_BarSize.y - m_EditorToolsHeight));

	if (ImGui::Begin("##Editor", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
	{
		if (ImGui::BeginTable("GXT2 Editor", 
			eColumnSetup::COLUMN_MAX, 
				ImGuiTableFlags_SizingFixedFit | 
				ImGuiTableFlags_RowBg          | 
				ImGuiTableFlags_BordersOuter   | 
				ImGuiTableFlags_BordersV       | 
				ImGuiTableFlags_Resizable      | 
				ImGuiTableFlags_ScrollX        | 
				ImGuiTableFlags_ScrollY        | 
				ImGuiTableFlags_Sortable))
		{
			ImGui::TableSetupColumn("", 
				ImGuiTableColumnFlags_NoHide    | 
				ImGuiTableColumnFlags_NoResize  |
				ImGuiTabItemFlags_NoReorder     | 
				ImGuiTableColumnFlags_NoSort, 
				10.f);
			ImGui::TableSetupColumn("Hash", ImGuiTableColumnFlags_NoHide, 150.f);
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
						// Data
						const unsigned int& uHash	= m_Filter[i].first;
						std::string& szText			= m_Filter[i].second;
						std::string& displayName	= m_LabelNames->GetData()[uHash];

#if 0
						const CFile::Map::const_iterator itMap = m_LabelNames->GetDataConst().find(uHash);
						if (itMap == m_LabelNames->GetDataConst().end())
						{
							UpdateDisplayName(uHash, true);
							m_SortViewNextRound = true;
						}
						std::string displayName = m_LabelNames->GetDataConst().find(uHash)->second;
#endif

						ImGui::TableNextRow();

						ImGui::TableSetColumnIndex(eColumnSetup::COLUMN_DELETE);
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - trashIconWidth) * 0.5f);
						if (ImGui::Button((ICON_FA_TRASH "##Delete" + displayName).c_str()))
						{
							FlagForDeletion(uHash);
						}
						ImGui::PopStyleColor();

						ImGui::TableSetColumnIndex(eColumnSetup::COLUMN_HASH);
						ImGui::PushItemWidth(-FLT_EPSILON);
						ImGui::InputText(("##Hash" + displayName).c_str(), &displayName, ImGuiInputTextFlags_ReadOnly);
						ImGui::PopItemWidth();

						ImGui::TableSetColumnIndex(eColumnSetup::COLUMN_TEXT);
						ImGui::PushItemWidth(-FLT_EPSILON);
						if (ImGui::InputText(("##Text" + displayName).c_str(), &szText, ImGuiInputTextFlags_AutoSelectAll))
						{
							if (szText.empty())
							{
								FlagForDeletion(uHash);
							}
							else
							{
								auto it = std::find_if(m_Data.begin(), m_Data.end(),
									[uHash](const std::pair<unsigned int, std::string>& entry) -> bool
									{
										return entry.first == uHash;
									});

								if (it != m_Data.end())
								{
									if (it->second != szText)
									{
										it->second = szText;
									}
								}
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
	ImGui::SetNextWindowPos(ImVec2(pViewport->Pos.x, pViewport->Size.y - m_EditorToolsHeight));
	ImGui::SetNextWindowSize(ImVec2(pViewport->Size.x, m_EditorToolsHeight));

	if (ImGui::Begin("Editor Bar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
	{
		static const float paddingLeftRight = 15.f;

		ImGui::SetCursorPosX(paddingLeftRight);
		ImGui::AlignTextToFramePadding();
		ImGui::Text(ICON_FA_CODE " Hash");
		ImGui::SameLine();

		ImGui::PushItemWidth(90.f);
		const float cursorX = ImGui::GetCursorPosX();
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

		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - paddingLeftRight - 105.f);
		ImGui::InputText("##TextInput", &m_TextInput);
		ImGui::PopItemWidth();
		ImGui::SameLine();

		if (ImGui::Button("Add", ImVec2(100.f, 0.f)))
		{
			if (!m_TextInput.empty())
			{
				const unsigned int uHash = strtoul(m_HashInput.c_str(), nullptr, 16);

				if (uHash != 0x00000000)
				{
					auto it = std::find_if(m_Data.begin(), m_Data.end(),
						[uHash](const std::pair<unsigned int, std::string>& entry) -> bool
						{
							return entry.first == uHash;
						});

					if (it != m_Data.end())
					{
						m_RenderEntryAlreadyExistPopup = true;
					}
					m_ItemsToAdd.push(std::make_pair(uHash, m_TextInput));
				}
				else
				{
					m_RenderEmptyHashPopup = true;
				}
			}
			else
			{
				m_RenderEmptyTextPopup = true;
			}
		}

		ImGui::Spacing();

		ImGui::SetCursorPosX(paddingLeftRight);
		ImGui::AlignTextToFramePadding();
		ImGui::Text(ICON_FA_MAGNIFYING_GLASS " Search");
		ImGui::SameLine();

		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - paddingLeftRight - 210.f);
		ImGui::SetCursorPosX(cursorX);
		ImGui::InputText("##SearchInput", &m_SearchInput);
		ImGui::PopItemWidth();
		ImGui::SameLine();

		if (ImGui::Button("Search", ImVec2(100.f, 0.f)))
		{
			if (!m_SearchInput.empty())
			{
				m_SortViewNextRound = true;
				UpdateFilter();
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Clear", ImVec2(100.f, 0.f)))
		{
			m_SearchInput.clear();
			m_SortViewNextRound = true;
			UpdateFilter();
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
			m_RequestCloseFile = false;
			CGraphics::SetIsClosing(false);
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	if (ImGui::BeginPopupModal("Override Entry", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("This entry already exists. Would you like to overwrite it?");
		ImGui::Separator();

		const float buttonWidth = 100.0f;
		const float buttonSpacing = ImGui::GetStyle().ItemSpacing.x;
		const float totalButtonWidth = buttonWidth * 2 + buttonSpacing;

		const float contentWidth = ImGui::GetContentRegionAvail().x;
		const float startX = (contentWidth - totalButtonWidth) / 2.0f;

		ImGui::SetCursorPosX(startX);

		if (ImGui::Button("Yes", ImVec2(buttonWidth, 0)))
		{
			m_OverrideExistingEntry = true;
			m_RenderEntryAlreadyExistPopup = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("No", ImVec2(buttonWidth, 0)))
		{
			m_ItemsToAdd.pop();
			m_RenderEntryAlreadyExistPopup = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	if (ImGui::BeginPopupModal("Hash Empty", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Hash input field is empty");
		ImGui::Separator();

		const float buttonWidth = 100.f;
		const float contentWidth = ImGui::GetContentRegionAvail().x;
		const float startX = (contentWidth - buttonWidth) * 0.5f;
		if (startX > 0.0f)
		{
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + startX);
		}

		if (ImGui::Button("Ok", ImVec2(buttonWidth, 0)))
		{
			m_RenderEmptyHashPopup = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
	
	if (ImGui::BeginPopupModal("Text Empty", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Text input field is empty");
		ImGui::Separator();

		const float buttonWidth = 100.f;
		const float contentWidth = ImGui::GetContentRegionAvail().x;
		const float startX = (contentWidth - buttonWidth) * 0.5f;
		if (startX > 0.0f)
		{
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + startX);
		}

		if (ImGui::Button("Ok", ImVec2(buttonWidth, 0)))
		{
			m_RenderEmptyTextPopup = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	if (m_RenderSaveChangesPopup)
	{
		ImGui::OpenPopup("Unsaved Changes");
	}
	if (m_RenderEntryAlreadyExistPopup)
	{
		ImGui::OpenPopup("Override Entry");
	}
	if (m_RenderEmptyHashPopup)
	{
		ImGui::OpenPopup("Hash Empty");
	}
	if (m_RenderEmptyTextPopup)
	{
		ImGui::OpenPopup("Text Empty");
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
	if (sortSpecs && (sortSpecs->SpecsDirty || m_SortViewNextRound))
	{
		auto compareEntries = [&](const std::pair<unsigned int, std::string>& a, const std::pair<unsigned int, std::string>& b) -> bool
		{
			for (int n = 0; n < sortSpecs->SpecsCount; n++)
			{
				int delta = 0;
				const ImGuiTableColumnSortSpecs* sortSpec = &sortSpecs->Specs[n];
				
				switch (sortSpec->ColumnIndex)
				{
				case eColumnSetup::COLUMN_HASH:
				{
					if (m_LabelsNotFound)
					{
						// Hash comparison
						if (a.first < b.first) delta = -1;
						if (a.first > b.first) delta = 1;
					}
					else
					{
						const CFile::Map& mLabels = m_LabelNames->GetDataConst();
						const CFile::Map::const_iterator aIt = mLabels.find(a.first);
						const CFile::Map::const_iterator bIt = mLabels.find(b.first);

						if (aIt != mLabels.end() && bIt != mLabels.end())
						{
							// Name comparison
							delta = aIt->second.compare(bIt->second);
						}
						else
						{
							// Hash comparison
							if (a.first < b.first) delta = -1;
							if (a.first > b.first) delta = 1;
						}
					}
				}
				break;
				case eColumnSetup::COLUMN_TEXT:
				{
					delta = a.second.compare(b.second);
				}
				break;
				}
				if (delta != 0)
					return (sortSpec->SortDirection == ImGuiSortDirection_Ascending) ? (delta < 0) : (delta > 0);
			}
			return false;
		};

		MEASURE_START;
		{
			if (m_SortUnderlyingData)
			{
				std::sort(std::execution::par, m_Data.begin(), m_Data.end(), compareEntries);
			}
			std::sort(std::execution::par, m_Filter.begin(), m_Filter.end(), compareEntries);
		}
		MEASURE_END;

#if MEASURE_ENABLED
		printf("[%s] Entry Count = %lli, m_SortViewNextRound = %s, m_SortUnderlyingData = %s, Execution Time = %lli ms\n", __FUNCTION__, 
			m_Filter.size(), 
			(m_SortViewNextRound) ? "true" : "false", 
			(m_SortUnderlyingData) ? "true" : "false",
			MEASURE_MS);
#endif

		sortSpecs->SpecsDirty = false;
		m_SortViewNextRound = false;
		m_SortUnderlyingData = false;
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

	if (utils::OpenFileExplorerDialog("Select a GTA Text Table", "", m_Path, false, { FILEDESC_GXT2, FILTERSPEC_GXT2 }))
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
		if (pInputDevice->ReadEntries())
		{
			for (const auto& [uHash, szEntry] : pInputDevice->GetData())
			{
				m_Data.emplace_back(uHash, szEntry);
			}
			m_Filter = m_Data;
			m_SortViewNextRound = true;
			m_SortUnderlyingData = true;

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

bool gxt2edit::CheckChanges()
{
	if (m_HasPendingChanges)
	{
		m_RenderSaveChangesPopup = true;
		return false;
	}
	return true;
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
		m_SortUnderlyingData = true;
		return;
	}
	for (const auto& entry : m_Data)
	{
		if (utils::ToLower(entry.second).find(utils::ToLower(m_SearchInput)) != std::string::npos)
		{
			m_Filter.push_back(entry);
		}
		else if (utils::ToLower(m_LabelNames->GetData()[entry.first]).find(utils::ToLower(m_SearchInput)) != std::string::npos)
		{
			m_Filter.push_back(entry);
		}
	}
}

void gxt2edit::UpdateEntries()
{
	bool bShouldUpdateFilter = false;

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

			// Update State
			bShouldUpdateFilter = true;
			m_HasPendingChanges = true;
		}
	}

	if (!m_ItemsToAdd.empty())
	{
		const auto& entryToAdd = m_ItemsToAdd.top();
		const unsigned int uHash = entryToAdd.first;

		// Entry is not a duplicate, insert and pop
		if (!m_RenderEntryAlreadyExistPopup && !m_OverrideExistingEntry)
		{
			m_Data.push_back(entryToAdd);
			m_ItemsToAdd.pop();

			// Update State
			bShouldUpdateFilter = true;
			m_HasPendingChanges = true;

			// Update Display Names
			UpdateDisplayName(uHash);

			// Insertion was successful, clear fields
			m_HashInput.clear();
			m_LabelInput.clear();
			m_TextInput.clear();
		}
		else if (m_OverrideExistingEntry) // User clicked "Yes" - override existing entry and pop it
		{
			auto it = std::find_if(m_Data.begin(), m_Data.end(),
				[uHash](const std::pair<unsigned int, std::string>& entry) -> bool
				{
					return entry.first == uHash;
				});

			if (it != m_Data.end())
			{
				// Override and Pop
				it->second = entryToAdd.second;
				m_ItemsToAdd.pop();

				// Update State
				bShouldUpdateFilter = true;
				m_HasPendingChanges = true;
				m_OverrideExistingEntry = false;

				// Update Display Names
				UpdateDisplayName(uHash);

				// Insertion was successful, clear fields
				m_HashInput.clear();
				m_LabelInput.clear();
				m_TextInput.clear();
			}
		}
#if _DEBUG && 0
		else
		{
			printf("Keeping Entry on Stack, Hash: 0x%08x\n", uHash);
			printf("m_RenderEntryAlreadyExistPopup = %i\n", m_RenderEntryAlreadyExistPopup);
		}
#endif
	}

	if (!m_EntriesToRemove.empty())
	{
		m_EntriesToRemove.clear();
	}
	if (bShouldUpdateFilter)
	{
		UpdateFilter();
	}
}

void gxt2edit::UpdateDisplayName(unsigned int uHash, bool bHashOnly /*= false*/)
{
	if (!m_LabelNames)
	{
		return;
	}

	if (auto it = m_LabelNames->GetData().find(uHash); it == m_LabelNames->GetData().end())
	{
		if (!m_LabelInput.empty() && !bHashOnly)
		{
			m_LabelNames->GetData()[uHash] = m_LabelInput;
		}
		else
		{
			m_LabelNames->GetData()[uHash] = std::format("0x{:08X}", uHash);
		}
	}
};

void gxt2edit::CacheDisplayNames()
{
	static bool bManageLabelNames = false;

	if (!m_Data.empty())
	{
		if (!bManageLabelNames)
		{
			for (const auto& [uHash, szTextEntry] : m_Data)
			{
				IM_UNUSED(szTextEntry);
				UpdateDisplayName(uHash, true);
			}
			bManageLabelNames = true;
		}
	}
	else
	{
		bManageLabelNames = false;
	}
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
	//#define MAIN_ENTRY_RELEASE
	//#define WINMAIN_ENTRY_DEBUG
	#if defined(_DEBUG) && !defined(WINMAIN_ENTRY_DEBUG) || defined(MAIN_ENTRY_RELEASE)
		#pragma comment(linker, "/SUBSYSTEM:CONSOLE")
	#else
		#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
	#endif
#endif

#if (defined(_DEBUG) && !defined(WINMAIN_ENTRY_DEBUG)) || !defined(_WIN32) || defined(MAIN_ENTRY_RELEASE)
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
#if defined(_WIN32) && !defined(MAIN_ENTRY_RELEASE) && (!defined(_DEBUG) || defined(WINMAIN_ENTRY_DEBUG))
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

#if defined(_WIN32) && !defined(MAIN_ENTRY_RELEASE) && (!defined(_DEBUG) || defined(WINMAIN_ENTRY_DEBUG))
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