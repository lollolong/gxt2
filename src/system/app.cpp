//
//	system/app.cpp
//

#ifdef UI_BACKEND

// Project
#include "app.h"

#define _glfw3_native_h_
#include "grc/graphics.h"

// C/C++
#include <thread>

CAppUI::CAppUI(const std::string& windowTitle, int width, int height) :
	m_WindowTitle(windowTitle),
	m_Width(width),
	m_Height(height)
{
}

int CAppUI::Run(int /*argc*/, char* /*argv*/[])
{
	const int bSuccess = Init();

	if (bSuccess)
	{
		while (CGraphics::GetInstance().IsRunning())
		{
			if (CGraphics::GetInstance().IsMinimized())
			{
				glfwPollEvents();
				std::this_thread::sleep_for(std::chrono::milliseconds(15));
			}
			else
			{
				OnTickInternal();
			}
		}
		Shutdown();
	}

	return bSuccess;
}

bool CAppUI::Init()
{
	return CGraphics::GetInstance().Init(m_WindowTitle, m_Width, m_Height);
}

void CAppUI::OnTickInternal()
{
	CGraphics::GetInstance().PreRender();
	OnTick();
	CGraphics::GetInstance().Render();
}

void CAppUI::Shutdown()
{
	CGraphics::GetInstance().Shutdown();
}

#endif // UI_BACKEND