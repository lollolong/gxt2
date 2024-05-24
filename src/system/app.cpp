//
//	system/app.cpp
//

#include "app.h"

#ifdef UI_BACKEND

// Project
#define _glfw3_native_h_
#include "grc/glfw_vulkan.h"

CAppUI::CAppUI(const string& windowTitle, int width, int height) :
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
			OnTickInternal();
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