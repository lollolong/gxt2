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


gxt2edit::gxt2edit(const string& windowTitle) : 
	CAppUI(windowTitle),
	m_Input(nullptr)
{

}

int gxt2edit::Run(int argc, char* argv[])
{
	if (argc == 2)
	{
		m_Input = new CGxt2File(argv[1]);
	}
	else
	{
		m_Input = new CFile();
	}
	

	m_Input->ReadEntries();

	return CAppUI::Run(argc, argv);
}

void gxt2edit::OnTick()
{
	const ImGuiViewport* pViewport = ImGui::GetMainViewport();

	{
		ImGui::SetNextWindowPos(pViewport->Pos);
		ImGui::SetNextWindowSize(pViewport->Size);
		ImGui::SetNextWindowSize(ImVec2(pViewport->Size.x * 2 / 3, pViewport->Size.y));

		ImGui::Begin("GXT Label Editor", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

		if (ImGui::BeginTable("GXT2 Editor", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY))
		{
			ImGui::TableSetupColumn("Hash", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoResize);
			ImGui::TableSetupColumn("Text", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupScrollFreeze(0, 1);
			ImGui::TableHeadersRow();

			ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.f, 0.f));
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, ImGui::GetStyle().FramePadding.y));

			ImGuiListClipper clipper;
			clipper.Begin(static_cast<int>(m_Input->GetData().size()), ImGui::GetTextLineHeightWithSpacing());

			while (clipper.Step())
			{
				for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
				{
					auto it = std::next(m_Input->GetData().begin(), i);

					const unsigned int& uHash = it->first;
					string& text = it->second;

					string szHash = format("0x{:08X}", uHash);

					ImGui::TableNextRow();


					// Hash column
					ImGui::TableSetColumnIndex(0);
					ImGui::PushItemWidth(90.f);
					ImGui::InputText(("##Hash" + szHash).c_str(), &szHash, ImGuiInputTextFlags_ReadOnly);
					ImGui::PopItemWidth();

					// Text column
					ImGui::TableSetColumnIndex(1);
					ImGui::PushItemWidth(-FLT_EPSILON);
					if (ImGui::InputText(("##Text" + std::to_string(uHash)).c_str(), &text, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
					{
						
					}
					ImGui::PopItemWidth();
				}
			}

			ImGui::PopStyleColor();
			ImGui::PopStyleVar();
			ImGui::PopStyleVar();

			ImGui::EndTable();
		}

		ImGui::End();
	}

	{
		ImGui::SetNextWindowPos(ImVec2(pViewport->Pos.x + pViewport->Size.x * 2 / 3, pViewport->Pos.y));
		ImGui::SetNextWindowSize(ImVec2(pViewport->Size.x * 1 / 3, pViewport->Size.y));
		if (ImGui::Begin("Add Row", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
		{
			
		}
		ImGui::End();
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
		gxt2edit gxt2edit("GXT2 Editor");
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