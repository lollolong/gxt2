//
//	system/app.h
//

#ifndef _APP_H_
#define _APP_H_

// C/C++
#include <string>

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
	CAppUI(const std::string& windowTitle, int width, int height);
	virtual ~CAppUI() = default;
public:
	int Run(int argc, char* argv[]) override;
private:
	virtual bool Init();
	virtual void OnTick() = 0;
	virtual void OnTickInternal();
	virtual void Shutdown();
private:
	std::string m_WindowTitle;
	int m_Width;
	int m_Height;
};

#endif // _APP_H_