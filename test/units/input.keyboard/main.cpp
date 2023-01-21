#include "rl/input.keyboard.hpp"
#include "rl/visualstyles.h"
#include "resource.h"

#include <atomic>
#include <thread>
#include <Windows.h>
#include <CommCtrl.h>

#pragma comment(lib, "Comctl32.lib")

void ThreadFunc();
LRESULT WINAPI MessageProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);



const int iWIDTH_CLIENT = 280;
const int iHEIGHT_CLIENT = 18;
const DWORD dwSTYLE = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;

std::atomic_bool bAtomActive = false;
HWND hWnd = NULL;

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR szCmdLine,
	_In_ int iCmdShow)
{
	const TCHAR szClassName[] = TEXT("TestWindowClass");

	WNDCLASS wc = { sizeof(WNDCLASS) };

	wc.lpszClassName = szClassName;
	wc.lpfnWndProc = MessageProc;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hInstance = hInstance;
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ROBINLE));
	wc.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClass(&wc))
	{
		MessageBox(NULL, TEXT("Couldn't register WNDCLASS"), NULL, MB_ICONERROR);
		return 1;
	}

	RECT rect = { 0, 0, iWIDTH_CLIENT, iHEIGHT_CLIENT };
	AdjustWindowRect(&rect, dwSTYLE, FALSE);

	const int iWIDTH_WINDOW = rect.right - rect.left;
	const int iHEIGHT_WINDOW = rect.bottom - rect.top;

	hWnd = CreateWindowEx(WS_EX_TOPMOST, szClassName,
		TEXT(R"(Test window for "input.keyboard")"), dwSTYLE, CW_USEDEFAULT, CW_USEDEFAULT,
		iWIDTH_WINDOW, iHEIGHT_WINDOW, NULL, NULL, NULL, NULL);

	if (hWnd == NULL)
	{
		MessageBox(NULL, TEXT("Couldn't create window"), NULL, MB_ICONERROR);
		return 1;
	}


	std::thread trd(ThreadFunc);

	MSG msg = {};
	while (true)
	{
		GetMessage(&msg, NULL, NULL, NULL);
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (msg.message == WM_QUIT)
			break;
	}

	bAtomActive = false;
	if (trd.joinable())
		trd.join();

	return 0;
}

void ThreadFunc()
{
	bAtomActive = true;

	auto& kb = rl::Keyboard::getInstance();



	while (bAtomActive)
	{
		kb.processInput();


		if (kb.getKey('A').pressed())
			MessageBoxA(hWnd, "\"A\" pressed.", "Keypress", MB_ICONINFORMATION | MB_APPLMODAL);

		if (kb.getKey(VK_CONTROL).held() && kb.getKey('S').held())
		{
			if (kb.getKey(VK_SHIFT).held())
				MessageBoxA(hWnd, R"("Save as" shortcut pressed)", "Keypress",
					MB_ICONINFORMATION | MB_APPLMODAL);
			else
				MessageBoxA(hWnd, R"("Save" shortcut pressed)", "Keypress",
					MB_ICONINFORMATION | MB_APPLMODAL);
		}
	}
}

LRESULT WINAPI MessageProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (rl::Keyboard::getInstance().update(uMsg, wParam, lParam))
		return 0;

	switch (uMsg)
	{
	case WM_CREATE:
		InitCommonControls();

		CreateWindow(WC_STATIC, TEXT(R"(This is a test window for "input.keyboard".)"),
			WS_VISIBLE | WS_CHILD | SS_CENTER | SS_NOPREFIX,
			0, 0, iWIDTH_CLIENT, iHEIGHT_CLIENT, hWnd, NULL, GetModuleHandle(NULL), NULL);

		break;

	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_QUIT:
		break;

	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}
