#include "resource.h"

#include "../../rlSplashScreen.hpp"
#include "../../rlOpenGLWin.hpp"

#include <Windows.h>




class GLTest : public rl::OpenGLWin
{
private:

	bool OnCreate() override
	{
		Sleep(5000); // emulate resource loading
		rl::SplashScreen::Close();
		return true;
	}

	bool OnUpdate(float fElapsedTime) override
	{
		return true;
	}

};




int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR szCmdLine,
	_In_ int iCmdShow)
{
	rl::SplashScreen_Config splashconfig(IDB_SPLASH);
	splashconfig.bDropShadow = true;
	//splashconfig.bAlwaysOnTop = true;
	
	rl::SplashScreen::Show(splashconfig);


	rl::OpenGLWin_Config openglconfig;
	//config.bInitialFullscreen = true;
	openglconfig.hIconBig = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_ROBINLE));

	GLTest gltest;
	gltest.run(openglconfig);


	return 0;
}
