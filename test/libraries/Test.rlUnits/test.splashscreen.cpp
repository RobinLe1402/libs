#include "tests.hpp"

// project
#include "resource.h"

// rl
#include <rl/splashscreen.hpp>



bool UnitTest_splashscreen()
{
	rl::SplashScreen_Config config;
	config.bAlwaysOnTop = true;
	config.bDropShadow = true;


	if (!rl::SplashScreen::ShowBitmap(config, IDB_SPLASH))
		return false;

	Sleep(5000);
	rl::SplashScreen::Close();

	return true;
}