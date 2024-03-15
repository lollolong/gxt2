//
//	system/app.h
//

#ifndef _APP_H_
#define _APP_H_

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
	virtual bool Init();
	virtual void OnTick() = 0;
	virtual void OnTickInternal();
	virtual void Shutdown();
private:
	string m_WindowTitle;
};

#endif // _APP_H_