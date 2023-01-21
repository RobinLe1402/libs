#include "rl/splashscreen.hpp"
#include "resource.h"

#include <Windows.h>



int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR szCmdLine,
	_In_ int iCmdShow)
{
	rl::SplashScreen_Config config;
	config.bAlwaysOnTop = true;
	config.bDropShadow = true;


	if (rl::SplashScreen::ShowBitmap(config, IDB_SPLASH))
		Sleep(5000);
	rl::SplashScreen::Close();

	return 0;
}
