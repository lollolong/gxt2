//
//	system/app.h
//

#ifndef _APP_H_
#define _APP_H_
//
//#include "imgui.h"
//
//// Project
//#include "grc/glfw_vulkan.h"



// C/C++
#include <string>

using namespace std;

class CApp
{
public:
	CApp() = default;
	virtual ~CApp() = default;
public:
	virtual int Run(int argc, char* argv[]) = 0;
};

class CAppUI : public CApp
{
public:
	CAppUI(const string& windowTitle);
	virtual ~CAppUI() = default;
public:
	int Run(int argc, char* argv[]) override;
private:
	virtual int Init();
	virtual void OnTick() = 0;
	virtual void OnTickInternal();
	virtual void Shutdown();
protected:
	bool IsRunning() const;
private:
	void* m_Window;
	string m_WindowTitle;
};

#endif // _APP_H_