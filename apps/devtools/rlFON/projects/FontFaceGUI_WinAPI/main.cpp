#include <Windows.h>
#include "gui.main.hpp"

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR szCmdLine,
	_In_ int iCmdShow)
{
	if (!CreateGUI(hInstance, szCmdLine, iCmdShow))
		return 1;


	RunGUI();


	return 0;
}
