#include "rl/lib/rlOpenGL_ST/Application.hpp"
#include "rl/lib/rlOpenGL_ST/Window.hpp"

#include <Windows.h>


namespace lib = rl::OpenGL_ST;



int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR szCmdLine,
	_In_ int iCmdShow)
{
	lib::WindowStartupConfig cfg;
	cfg.bVisible = true;

	lib::Window oWindow(L"TestWndClass", cfg);
	lib::Window oWin2(L"TestWndClass2", cfg);

	lib::ThisApplication.addWindow(oWindow);
	lib::ThisApplication.addWindow(oWin2);
	lib::ThisApplication.run();

	return 0;
}
