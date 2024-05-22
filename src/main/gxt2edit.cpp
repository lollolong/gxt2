//
//	main/gxt2edit.cpp
//

// Windows
#include <Windows.h>

// Project
#include "gxt2edit.h"
#include "grc/glfw_vulkan.h"

// C/C++
#include <vector>
#include <format>

// vendor
#include <IconsFontAwesome6.h>

gxt2edit::gxt2edit(const string& windowTitle, int width, int height) :
	CAppUI(windowTitle, width, height),
	m_Input(nullptr)
{

}

gxt2edit::~gxt2edit()
{
	if (m_Input)
	{
		delete m_Input;
		m_Input = nullptr;
	}
}

int gxt2edit::Run(int argc, char* argv[])
{
	if (argc == 2)
	{
		m_Input = new CGxt2File(argv[1]);
		m_Input->ReadEntries();
	}
	return CAppUI::Run(argc, argv);
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
			}
			if (ImGui::MenuItem(ICON_FA_FILE "  New File"))
			{

			}
			if (ImGui::MenuItem(ICON_FA_FLOPPY_DISK "  Save"))
			{

			}
			if (ImGui::MenuItem(ICON_FA_COPY "  Save As"))
			{

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
		ImGui::SetNextWindowSize(pViewport->Size);

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

				if (m_Input)
				{
					ImGuiListClipper clipper;
					clipper.Begin(static_cast<int>(m_Input->GetData().size()) /*, ImGui::GetTextLineHeightWithSpacing()*/);

					while (clipper.Step())
					{
						for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
						{
							auto it = std::next(m_Input->GetData().begin(), i);

							const unsigned int& uHash = it->first;
							string& text = it->second;

							string szHash = format("0x{:08X}", uHash);

							ImGui::TableNextRow();

							// Delete column
							ImGui::TableSetColumnIndex(0);
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(ICON_FA_TRASH).x) * 0.5f);
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
						Sleep(15);
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

	UpdateEntries();
}

void gxt2edit::FlagForDeletion(unsigned int uHash)
{
	m_EntriesToRemove.push_back(uHash);
}

void gxt2edit::UpdateEntries()
{
	for (const unsigned int& uHash : m_EntriesToRemove)
	{
		m_Input->GetData().erase(uHash);
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