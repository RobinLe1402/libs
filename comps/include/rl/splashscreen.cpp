#include "splashscreen.hpp"

#include <atomic>
#include <thread>
#include <Windows.h>

// DWM
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

// GDI+
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")


/*

Borderless drop shadow uses code suggested by Stack Overflow user zett42
zett42:			https://stackoverflow.com/users/7571258/zett42
Code source:	https://stackoverflow.com/a/43819334

*/





namespace rl
{

	//==============================================================================================
	// STATIC VARIABLES

	bool SplashScreen::m_bShowing = false;
	std::atomic<bool> SplashScreen::m_bAtomShow = false;
	std::thread SplashScreen::m_trd;
	HBITMAP SplashScreen::m_hBmp = NULL;
	const wchar_t SplashScreen::m_szWinClassName[] = L"rlSplashScreen";
	bool SplashScreen::m_bDropShadow = false;





	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// STATIC METHODS

	bool SplashScreen::Show(SplashScreen_Config config)
	{
		if (m_bShowing)
			return false;

		while (GetLastError() != 0); // clear last error

		if (config.hMonitor == NULL)
			config.hMonitor = MonitorFromPoint({ 0, 0 }, MONITOR_DEFAULTTOPRIMARY);

		ULONG_PTR gpToken = NULL;
		Gdiplus::GdiplusStartupInput gpSi = NULL;
		Gdiplus::GdiplusStartup(&gpToken, &gpSi, NULL);

		Gdiplus::Bitmap* bmp = Gdiplus::Bitmap::FromResource(GetModuleHandle(NULL),
			MAKEINTRESOURCEW(config.iBitmapID));
		if (GetLastError() != 0)
		{
			delete bmp;
			Gdiplus::GdiplusShutdown(gpToken);
			MessageBoxA(NULL, "Couldn't load bitmap resource", "rlSplashScreen",
				MB_SYSTEMMODAL | MB_ICONERROR | MB_OK);
			return false;
		}

		m_bShowing = true;

		Gdiplus::Color clBg;
		clBg.SetValue(0xFFFFFFFF); // white background
		bmp->GetHBITMAP(clBg, &m_hBmp);
		unsigned int iWidth = bmp->GetWidth();
		unsigned int iHeight = bmp->GetHeight();
		delete bmp;
		Gdiplus::GdiplusShutdown(gpToken);

		WNDCLASSW wc = {};
		wc.lpfnWndProc = &WindowProc;
		wc.lpszClassName = m_szWinClassName;
		wc.hCursor = LoadCursorW(NULL, IDC_WAIT);
		wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

		RegisterClass(&wc);

		m_bAtomShow = true;
		m_trd = std::thread(&rl::SplashScreen::threadFunc, config, iWidth, iHeight);

		return true;
	}

	void SplashScreen::Close()
	{
		if (!m_bShowing)
			return;

		// thread is running --> close thread
		if (m_bAtomShow)
		{
			m_bAtomShow = false;
			m_trd.join();

			UnregisterClassW(m_szWinClassName, NULL);
			DeleteObject(m_hBmp);
		}

		// shutdown GDI+
		//Gdiplus::GdiplusShutdown(m_gpTk);

		m_bShowing = false;
	}





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	SplashScreen::~SplashScreen() { Close(); }

	void SplashScreen::threadFunc(SplashScreen_Config config, unsigned int iWidth, unsigned int iHeight)
	{
		DWORD dwExStyle = WS_EX_TOOLWINDOW;
		if (config.bAlwaysOnTop)
			dwExStyle |= WS_EX_TOPMOST;

		DWORD dwStyle;
		if (!config.bDropShadow)
			dwStyle = WS_POPUPWINDOW | WS_VISIBLE;
		else
			dwStyle = WS_CAPTION | WS_POPUP;

		m_bDropShadow = config.bDropShadow;

		MONITORINFO mi { sizeof(mi) };
		GetMonitorInfoW(config.hMonitor, &mi);

		int iScreenWidth = mi.rcWork.right - mi.rcWork.left;
		int iScreenHeight = mi.rcWork.bottom - mi.rcWork.top;
		int PosX = (int)(mi.rcWork.left + iScreenWidth / 2.0 - iWidth / 2.0);
		int PosY = (int)(mi.rcWork.top + iScreenHeight / 2.0 - iHeight / 2.0);

		HWND hWnd = CreateWindowExW(dwExStyle, m_szWinClassName, NULL, dwStyle, PosX, PosY,
			iWidth, iHeight, NULL, NULL, NULL, NULL);

		if (config.bDropShadow)
		{
			MARGINS margins = { 0, 0, 0, 1 };
			DwmExtendFrameIntoClientArea(hWnd, &margins);
			SetWindowPos(hWnd, NULL, 0, 0, 0, 0,
				SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

			ShowWindow(hWnd, SW_SHOW);
		}
		

		MSG msg;
		while (true)
		{
			if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}

			if (msg.message == WM_QUIT)
				break;

			if (!m_bAtomShow)
				DestroyWindow(hWnd);
		}
	}

	LRESULT SplashScreen::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_NCCALCSIZE:
			if (m_bDropShadow && wParam == TRUE)
				return 0;
			break;

		case WM_PAINT:
		{
			PAINTSTRUCT     ps;
			HDC             hdc;
			BITMAP          bitmap;
			HDC             hdcMem;
			HGDIOBJ         oldBitmap;

			hdc = BeginPaint(hWnd, &ps);

			hdcMem = CreateCompatibleDC(hdc);
			oldBitmap = SelectObject(hdcMem, m_hBmp);

			GetObject(m_hBmp, sizeof(bitmap), &bitmap);
			BitBlt(hdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);

			SelectObject(hdcMem, oldBitmap);
			DeleteDC(hdcMem);

			EndPaint(hWnd, &ps);
			return 0;
		}

		case WM_CLOSE: // disallow window closing by user
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		}

		return DefWindowProcW(hWnd, uMsg, wParam, lParam);
	}

}