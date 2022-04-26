// libraries
#pragma comment(lib, "rlOpenGL.lib")

// RobinLe includes
#include "rl/lib/rlOpenGL/Core.hpp"
#include "rl/splashscreen.hpp"

// STL includes
#include <Windows.h>


// project includes
#include "include/resource.h"
#include "include/application.hpp"
#include "include/renderer.hpp"
#include "include/window.hpp"


namespace gl = rl::OpenGL;







int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR szCmdLine,
	_In_ int iCmdShow)
{
	CustomWindow oWindow;
	Renderer oRenderer;
	Application oApplication(oWindow, oRenderer);

	gl::AppConfig cfg;
	cfg.window.bResizable = true;
	cfg.window.iMinWidth = 500;
	cfg.window.iWidth = cfg.window.iMinWidth;
	cfg.window.iMinHeight = 250;
	cfg.window.iHeight = cfg.window.iMinHeight;
	cfg.window.sTitle = L"RobinLe Font Designer";

	rl::SplashScreen_Config splash_cfg(IDB_SPLASH);
	splash_cfg.bAlwaysOnTop = true;
	splash_cfg.bDropShadow = true;
	rl::SplashScreen::Show(splash_cfg);

	if (!oApplication.execute(cfg))
		return 1;

	return 0;
}
