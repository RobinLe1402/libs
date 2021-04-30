#include "resource.h"

#include "../../rlSplashScreen.hpp"
#include "../../rlOpenGLWin.hpp"

#include <Windows.h>




class GLTest : public rl::OpenGLWin
{
private:

	bool OnCreate() override
	{
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
	
	rl::SplashScreen::Show(splashconfig);


	rl::OpenGLWin_Config config;
	config.bInitialFullscreen = true;

	GLTest gltest;
	gltest.run(config);


	return 0;
}
