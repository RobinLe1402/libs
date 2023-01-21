#include "rl/splashscreen.hpp"

#include <atomic>
#include <thread>
#include <Windows.h>

// DWM
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

// GDI+
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

// ShlWAPI
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")


/*

Borderless drop shadow uses code suggested by Stack Overflow user zett42
zett42:			https://stackoverflow.com/users/7571258/zett42
Code source:	https://stackoverflow.com/a/43819334

*/





namespace rl
{

	//==============================================================================================
	// STATIC VARIABLES

	const wchar_t SplashScreen::s_szWinClassName[] = L"rlSplashScreen";
	SplashScreen SplashScreen::s_oSplash;





	//==============================================================================================
	// METHODS

	Gdiplus::Bitmap* LoadImageFromResource(HMODULE hMod, const wchar_t* ResID,
		const wchar_t* ResType)
	{
		Gdiplus::Bitmap* pBmp = nullptr;

		HRSRC hRsrc = FindResourceW(hMod, ResID, ResType);
		if (hRsrc == NULL)
			return nullptr;

		DWORD dwResourceSize = SizeofResource(hMod, hRsrc);
		if (dwResourceSize == 0)
			return nullptr;

		HGLOBAL hGlobalResource = LoadResource(hMod, hRsrc);
		if (hGlobalResource == NULL)
			return nullptr;

		void* pData = LockResource(hGlobalResource);
		auto pStream = SHCreateMemStream(reinterpret_cast<const BYTE*>(pData),
			dwResourceSize);

		if (pStream == NULL)
			return nullptr;

		pBmp = new Gdiplus::Bitmap(pStream);
		pStream->Release();
		pStream = nullptr;
		return pBmp;
	}


	//----------------------------------------------------------------------------------------------
	// STATIC METHODS

	bool SplashScreen::ShowBitmap(const SplashScreen_Config& cfg, int iBitmapID)
	{
		return ShowImage(cfg, RT_BITMAP, iBitmapID);
	}

	bool SplashScreen::ShowImage(const SplashScreen_Config& cfg, const wchar_t* ResType, int iResID)
	{
		if (s_oSplash.m_bVisible)
			return false;

		SetLastError(0); // clear last error

		s_oSplash.m_oCfg = cfg;

		if (s_oSplash.m_oCfg.hMonitor == NULL)
			s_oSplash.m_oCfg.hMonitor = MonitorFromPoint({ 0, 0 }, MONITOR_DEFAULTTOPRIMARY);



		ULONG_PTR gpToken = NULL;
		Gdiplus::GdiplusStartupInput gpSi = NULL;
		Gdiplus::GdiplusStartup(&gpToken, &gpSi, NULL);

		Gdiplus::Bitmap* pBitmap = nullptr;
		if (ResType == RT_BITMAP)
			pBitmap = Gdiplus::Bitmap::FromResource(GetModuleHandle(NULL), MAKEINTRESOURCE(iResID));
		else
			pBitmap = LoadImageFromResource(GetModuleHandle(NULL), MAKEINTRESOURCE(iResID), ResType);
		Gdiplus::Color clBG;
		clBG.SetValue(0xFF000000 | cfg.iBackgroundColor);
		pBitmap->GetHBITMAP(clBG, &s_oSplash.m_hBmp);
		s_oSplash.m_iWidth = pBitmap->GetWidth();
		s_oSplash.m_iHeight = pBitmap->GetHeight();

		Gdiplus::GdiplusShutdown(gpToken);



		s_oSplash.m_bVisible = true;

		bool bResult = s_oSplash.showInternal();

		return bResult;
	}

	void SplashScreen::Close() { s_oSplash.closeInternal(); }





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	LRESULT SplashScreen::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_NCCALCSIZE:
			if (s_oSplash.m_oCfg.bDropShadow && wParam == TRUE)
				return 0;
			break;

		case WM_PAINT:
		{
			PAINTSTRUCT     ps;
			HDC             hdc;
			BITMAP          bitmap = {};
			HDC             hdcMem;
			HGDIOBJ         oldBitmap;

			hdc = BeginPaint(hWnd, &ps);

			hdcMem = CreateCompatibleDC(hdc);
			oldBitmap = SelectObject(hdcMem, s_oSplash.m_hBmp);

			GetObject(s_oSplash.m_hBmp, sizeof(bitmap), &bitmap);
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

	SplashScreen::~SplashScreen() { Close(); }

	void SplashScreen::threadFunc()
	{
		std::unique_lock lm(m_mux);
		lm.unlock();

		m_bRunning = false;

		DWORD dwExStyle = WS_EX_TOOLWINDOW;
		if (m_oCfg.bAlwaysOnTop)
			dwExStyle |= WS_EX_TOPMOST;

		DWORD dwStyle;
		if (!m_oCfg.bDropShadow)
			dwStyle = WS_POPUPWINDOW | WS_VISIBLE;
		else
			dwStyle = WS_CAPTION | WS_POPUP;

		MONITORINFO mi{ sizeof(mi) };
		GetMonitorInfoW(m_oCfg.hMonitor, &mi);

		int iScreenWidth = mi.rcWork.right - mi.rcWork.left;
		int iScreenHeight = mi.rcWork.bottom - mi.rcWork.top;
		int PosX = (int)(mi.rcWork.left + iScreenWidth / 2.0 - m_iWidth / 2.0);
		int PosY = (int)(mi.rcWork.top + iScreenHeight / 2.0 - m_iHeight / 2.0);

		HWND hWnd = CreateWindowExW(dwExStyle, s_szWinClassName, NULL, dwStyle, PosX, PosY,
			m_iWidth, m_iHeight, NULL, NULL, NULL, NULL);

		if (hWnd == NULL)
		{
			m_bRunning = false;
			m_cv.notify_one();
			return;
		}

		if (m_oCfg.bDropShadow)
		{
			MARGINS margins = { 0, 0, 0, 1 };
			DwmExtendFrameIntoClientArea(hWnd, &margins);
			SetWindowPos(hWnd, NULL, 0, 0, 0, 0,
				SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

			ShowWindow(hWnd, SW_SHOW);
		}

		m_bRunning = true;
		m_cv.notify_one();


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

			std::unique_lock lm(m_mux);
			if (!s_oSplash.m_bRunning)
				DestroyWindow(hWnd);
		}
		m_bVisible = false;
	}

	bool SplashScreen::showInternal()
	{
		WNDCLASSW wc = {};
		wc.lpfnWndProc = &SplashScreen::WindowProc;
		wc.lpszClassName = s_szWinClassName;
		wc.hCursor = LoadCursorW(NULL, IDC_WAIT);
		wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

		RegisterClass(&wc);

		std::unique_lock lm(m_mux);
		m_trd = std::thread(&SplashScreen::threadFunc, this);
		m_cv.wait(lm);
		m_bVisible = m_bRunning;
		return m_bRunning;
	}

	void SplashScreen::closeInternal()
	{
		if (!m_bVisible)
			return;

		std::unique_lock lm(m_mux);

		m_bRunning = false;
		lm.unlock();
		if (m_trd.joinable())
			m_trd.join();

		UnregisterClassW(s_szWinClassName, NULL);
		DeleteObject(m_hBmp);
	}

}