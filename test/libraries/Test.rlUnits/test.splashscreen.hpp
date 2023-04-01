#pragma once
#ifndef ROBINLE_UNITTEST_SPLASHSCREEN
#define ROBINLE_UNITTEST_SPLASHSCREEN



#include "rl/splashscreen.hpp"
#include "resource.h"



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



#endif // ROBINLE_UNITTEST_SPLASHSCREEN