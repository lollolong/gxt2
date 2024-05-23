//
//	main/gxt2edit.cpp
//

// Windows
#include <Windows.h>

// Project
#include "gxt2edit.h"
#include "data/util.h"
#include "data/stringhash.h"
#include "grc/glfw_vulkan.h"

// C/C++
#include <vector>
#include <format>

// vendor
#include <IconsFontAwesome6.h>

gxt2edit::gxt2edit(const string& windowTitle, int width, int height) :
	CAppUI(windowTitle, width, height)
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

void gxt2edit::SaveToFile(const string& path, eFileType fileType)
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
		delete pOutputDevice;
	}
}

void gxt2edit::LoadFromFile(const string& path, eFileType fileType)
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
	default:
		break;
	}

	if (pInputDevice)
	{
		pInputDevice->SetData(m_Data);
		if (pInputDevice->ReadEntries())
		{
			m_Data = pInputDevice->GetData();
			if (fileType == FILETYPE_GXT2)
			{
				m_Path = path;
			}
		}
		delete pInputDevice;
	}
}

void gxt2edit::OnTick()
{
	m_EntriesToRemove.clear();

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem(ICON_FA_FOLDER_OPEN " Open File"))
			{
				if (utils::OpenFileExplorerDialog(NULL, L"Select a GTA Text Table", L"", m_Path, false, { { L"GTA Text Table (*.gxt2)", L"*.gxt2" } }))
				{
					Reset();
					LoadFromFile(m_Path, FILETYPE_GXT2);
				}
			}
			if (ImGui::MenuItem(ICON_FA_FILE "  New File"))
			{
				Reset();
			}
			if (ImGui::MenuItem(ICON_FA_FLOPPY_DISK "  Save"))
			{
				SaveFile();
			}
			if (ImGui::MenuItem(ICON_FA_COPY "  Save As"))
			{
				SaveFileAs();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Export and Import"))
		{
			if (ImGui::MenuItem(ICON_FA_FILE_EXPORT "Export"))
			{
				if (utils::OpenFileExplorerDialog(NULL, L"Export Text Table (JSON, TXT ...)", L"", m_Path, true,
					{
						{ L"JSON File (*.json)", L"*.json" },
						{ L"Text File (*.txt)", L"*.txt" },
					}
				))
				{
					eFileType fileType = FILETYPE_UNKNOWN;
					const string szInputExtension = m_Path.substr(m_Path.find_last_of("."));

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

					if (fileType != FILETYPE_UNKNOWN)
					{
						SaveToFile(m_Path, fileType);
					}
				}
			}
			if (ImGui::MenuItem(ICON_FA_FILE_IMPORT "Import"))
			{
				if (utils::OpenFileExplorerDialog(NULL, L"Import Text Table (JSON, TXT ...)", L"", m_Path, false, 
					{ 
						{ L"JSON File (*.json)", L"*.json" },
						{ L"Text File (*.txt)", L"*.txt" },
					}
				))
				{
					eFileType fileType = FILETYPE_UNKNOWN;
					const string szInputExtension = m_Path.substr(m_Path.find_last_of("."));

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

					if (fileType != FILETYPE_UNKNOWN)
					{
						Reset();
						LoadFromFile(m_Path, fileType);
					}
				}
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Settings"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				exit(0);
			}
			ImGui::EndMenu();
		}

		m_BarSize = ImGui::GetWindowSize();
		ImGui::EndMainMenuBar();
	}

	{
		const ImGuiViewport* pViewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(ImVec2(pViewport->Pos.x, m_BarSize.y));
		ImGui::SetNextWindowSize(ImVec2(pViewport->Size.x, pViewport->Size.y - m_BarSize.y - 65.f));

		if (ImGui::Begin("##Editor", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
		{
			if (ImGui::BeginTable("GXT2 Editor", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY))
			{
				ImGui::TableSetupColumn("", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoResize | ImGuiTabItemFlags_NoReorder);
				ImGui::TableSetupColumn("Hash", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoResize);
				ImGui::TableSetupColumn("Text", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupScrollFreeze(0, 1);
				ImGui::TableHeadersRow();

				ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
				ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.f, 0.f));
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, ImGui::GetStyle().FramePadding.y));

				if (!m_Data.empty())
				{
					ImGuiListClipper clipper;
					clipper.Begin(static_cast<int>(m_Data.size()) /*, ImGui::GetTextLineHeightWithSpacing()*/);

					const float trashIconWidth = ImGui::CalcTextSize(ICON_FA_TRASH).x;

					while (clipper.Step())
					{
						CFile::Map::iterator it = std::next(m_Data.begin(), clipper.DisplayStart);
						for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i, ++it)
						{
							const unsigned int& uHash = it->first;
							string& text = it->second;

							string szHash = format("0x{:08X}", uHash);

							ImGui::TableNextRow();

							// Delete column
							ImGui::TableSetColumnIndex(0);
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - trashIconWidth) * 0.5f);
							if (ImGui::Button((ICON_FA_TRASH "##" + to_string(uHash)).c_str()))
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
							if (ImGui::InputText(("##Text" + std::to_string(uHash)).c_str(), &text, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
							{
								if (text.empty())
								{
									FlagForDeletion(uHash);
								}
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
				m_HashInput = format("{:08X}", rage::atStringHash(m_LabelInput.c_str()));
			}
			ImGui::PopItemWidth();
			ImGui::SameLine();

			ImGui::Text(ICON_FA_PENCIL " Text");
			ImGui::SameLine();

			float remainingWidth = ImGui::GetContentRegionAvail().x - 50.f; // Adjusted to leave space for the Add button
			ImGui::PushItemWidth(remainingWidth);
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
						m_Data[uHash] = m_TextInput;

						m_HashInput.clear();
						m_LabelInput.clear();
						m_TextInput.clear();
					}
				}
			}

			ImGui::End();
		}
	}
	UpdateEntries();
}

void gxt2edit::OpenFile()
{
	if (utils::OpenFileExplorerDialog(NULL, L"Select a GTA Text Table", L"", m_Path, false, { { L"GTA Text Table (*.gxt2)", L"*.gxt2" } }))
	{
		Reset();
		LoadFromFile(m_Path, FILETYPE_JSON);
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
	if (utils::OpenFileExplorerDialog(NULL, L"Save GTA Text Table", L"global.gxt2", m_Path, true, { { L"GTA Text Table (*.gxt2)", L"*.gxt2" } }))
	{
		SaveToFile(m_Path, FILETYPE_GXT2);
	}
}

void gxt2edit::FlagForDeletion(unsigned int uHash)
{
	m_EntriesToRemove.push_back(uHash);
}

void gxt2edit::UpdateEntries()
{
	for (const unsigned int& uHash : m_EntriesToRemove)
	{
		m_Data.erase(uHash);
	}
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
		gxt2edit gxt2edit("GXT2 Editor", 1500, 850);
		gxt2edit.Run(argc, argv);
	}
	catch (const exception& ex)
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