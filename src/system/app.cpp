//
//	system/app.cpp
//

#include "app.h"

#ifdef UI_BACKEND

#include "grc/glfw_vulkan.h"

CAppUI::CAppUI(const string& windowTitle) : 
	m_WindowTitle(windowTitle)
{

}

int CAppUI::Run(int /*argc*/, char* /*argv*/[])
{
	const int bSuccess = Init();
	
	if (bSuccess)
	{
		while (CGraphics::IsRunning())
		{
			OnTickInternal();
		}
		Shutdown();
	}

	return bSuccess;
}

bool CAppUI::Init()
{
	return CGraphics::Init(m_WindowTitle, 1225, 640);
}

void CAppUI::OnTickInternal()
{
	CGraphics::PreRender();
	OnTick();
	CGraphics::Render();
}

void CAppUI::Shutdown()
{
	CGraphics::Shutdown();
}

#endif // UI_BACKEND