#include "rlOpenGLWin.hpp"

#define NULL 0
#include <chrono>
#undef NULL

#include <stdint.h>
#include <thread>
#include <Windows.h>

// OpenGL
#include <GL/gl.h>
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "OpenGL32.lib")

// DWM
#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")



typedef BOOL(WINAPI wglSwapInterval_t)(int interval);





namespace rl
{

	//==============================================================================================
	// STATIC VARIABLES

	OpenGLWin* OpenGLWin::m_pInstance = nullptr;
	std::atomic<bool> OpenGLWin::m_bRunning = false;
	DWORD OpenGLWin::m_dwStyleCache = 0;





	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	OpenGLWin::OpenGLWin() {}
	OpenGLWin::~OpenGLWin() { quit(); }





	//----------------------------------------------------------------------------------------------
	// STATIC METHODS

	LRESULT OpenGLWin::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		static RECT rect = {};
		static bool bUnknownSize = false;

		bool bProcessed = true;
		switch (uMsg)
		{

			//--------------------------------------------------------------------------------------
			// MINIMUM/MAXIMUM SIZE

		case WM_GETMINMAXINFO:
			if (m_pInstance->m_bFullscreen)
			{
				bProcessed = false;
				break;
			}
			m_pInstance->setMinMaxStruct(lParam);
			break;



			//--------------------------------------------------------------------------------------
			// SIZE CHANGE/REPAINT

		case WM_SIZING:
			bUnknownSize = true;
			break;

		case WM_PAINT:
		{
			// only process message if window is being sized, isn't redrawn by Windows
			if (!bUnknownSize)
			{
				bProcessed = false;
				break;
			}

			// update size
			GetClientRect(hWnd, &rect);
			m_pInstance->m_iWidth = rect.right - rect.left;
			m_pInstance->m_iHeight = rect.bottom - rect.top;

			// repaint full window in black to prevent corrupted image being displayed
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			HBRUSH brush = CreateSolidBrush(0);
			FillRect(hdc, &ps.rcPaint, brush);
			DeleteObject(brush);
			EndPaint(hWnd, &ps);
			bUnknownSize = false;

			break;
		}

		case WM_SIZE:
			switch (wParam)
			{
			case SIZE_MINIMIZED:
				if (!m_pInstance->m_bMinimized)
				{
					m_pInstance->OnMinimize();
					m_pInstance->processMinimize();
				}
				break;
			case SIZE_RESTORED:
				if (m_pInstance->m_bMinimized)
				{
					m_pInstance->OnRestore();
					m_pInstance->processRestore();
				}
				break;
			}
			if (wParam != SIZE_MINIMIZED)
			{
				m_pInstance->m_iWidth = LOWORD(lParam);
				m_pInstance->m_iHeight = HIWORD(lParam);
			}
			break;



			//--------------------------------------------------------------------------------------
			// FOCUS

		case WM_KILLFOCUS: // minimize window when losing focus in fullscreen mode
			if (m_pInstance->m_bFullscreen)
				m_pInstance->minimize();
			break;

		case WM_SETFOCUS: // restore window when receiving focus in fullscreen mode
			if (m_pInstance->m_bFullscreen)
				m_pInstance->restore();
			break;



			//--------------------------------------------------------------------------------------
			// CLOSING

		case WM_CLOSE:
			// request destruction from thread
			m_pInstance->m_bAtomRunning = false;
			m_pInstance->m_bAtomThreadConfirmRunning = true;

			while (m_pInstance->m_bAtomThreadConfirmRunning); // wait for confirmation

			if (m_pInstance->m_bAtomRunning)
				return 0;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;



		default:
			bProcessed = false;
		}


		bProcessed |= m_pInstance->OnMessage(uMsg, wParam, lParam);

		// either return 0 ("no error") or call default procedure, based on if message was processed
		if (bProcessed)
			return 0;
		return DefWindowProcW(hWnd, uMsg, wParam, lParam);
	}





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	void OpenGLWin::run(OpenGLWin_Config config)
	{
		if (m_bRunning)
			return;

		// plausibility check: Min/Max sizes
		if (config.iWidthMin > 0)
		{
			if (config.iWidth < config.iWidthMin ||
				(config.iWidthMax > 0 && config.iWidthMin > config.iWidthMax))
				return;
		}
		if (config.iHeightMin > 0)
		{
			if (config.iHeight < config.iHeightMin ||
				config.iHeightMax > 0 && config.iHeightMin > config.iHeightMax)
				return;
		}
		if (config.iWidthMax > 0)
		{
			if (config.iWidth > config.iWidthMax)
				return;
		}
		if (config.iHeightMax > 0)
		{
			if (config.iHeight > config.iHeightMax)
				return;
		}

		// apply settings
		m_iWinWidth = config.iWidth;
		m_iWinHeight = config.iHeight;
		m_bResizable = config.bResizable;
		m_bFullscreen = config.bInitialFullscreen;
		m_bAtomVSync = config.bVSync;
		m_iWidthMin = config.iWidthMin;
		m_iHeightMin = config.iHeightMin;
		m_iWidthMax = config.iWidthMax;
		m_iHeightMax = config.iHeightMax;
		wcscat_s(m_szWinClassName, config.szWinClassName);
		m_hIconBig = config.hIconBig;
		m_hIconSmall = config.hIconSmall;
		m_hMonitorFullscreen = config.hMonitorFullscreen;

		// process settings
		if (m_hMonitorFullscreen == NULL)
		{
			const POINT pt = { 0, 0 };
			m_hMonitorFullscreen = MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);
		}
		if (m_bFullscreen)
		{
			MONITORINFO mi = { sizeof(mi) };
			GetMonitorInfoW(m_hMonitorFullscreen, &mi);
			m_iWidth = mi.rcMonitor.right - mi.rcMonitor.left;
			m_iHeight = mi.rcMonitor.bottom - mi.rcMonitor.top;
		}
		else
		{
			m_iWidth = m_iWinWidth;
			m_iHeight = m_iWinHeight;
		}



		WNDCLASSEXW wc = {};
		wc.cbSize = sizeof(wc);
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wc.lpfnWndProc = WindowProc;
		wc.lpszClassName = m_szWinClassName;
		wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
		wc.hIcon = m_hIconBig;
		wc.hIconSm = m_hIconSmall;

		auto a = RegisterClassExW(&wc);

		DWORD dwStyle = 0;
		if (!m_bFullscreen)
		{
			dwStyle |= WS_OVERLAPPEDWINDOW;

			if (!m_bResizable)
				dwStyle &= ~(WS_MAXIMIZEBOX | WS_THICKFRAME);
		}
		else
			dwStyle |= WS_POPUP;

		int iPos;
		if (m_bFullscreen)
			iPos = 0;
		else
			iPos = CW_USEDEFAULT;

		m_pInstance = this;

		int iWidth = m_iWidth;
		int iHeight = m_iHeight;

		if (!m_bFullscreen)
		{
			RECT rect = { 0, 0, iWidth, iHeight };
			AdjustWindowRect(&rect, dwStyle, FALSE);
			iWidth = rect.right - rect.left;
			iHeight = rect.bottom - rect.top;
		}
		m_dwStyleCache = dwStyle;
		m_hWnd = CreateWindowW(m_szWinClassName, config.szInitialCaption, dwStyle, iPos,
			iPos, iWidth, iHeight, NULL, NULL, NULL, NULL);
		m_dwStyleCache = 0;

		m_bAtomThreadConfirmRunning = true;

		std::thread trd(&rl::OpenGLWin::OpenGLThread, this, GetDC(m_hWnd));

		while (m_bAtomThreadConfirmRunning); // wait for thread to give startup permission (or not)

		if (m_bAtomRunning)
		{
			m_bRunning = true;
			m_bAtomRunning = true;

			auto time1 = std::chrono::system_clock::now();
			auto time2 = time1;

			//repaint();
			ShowWindow(m_hWnd, SW_SHOW);

			MSG msg{};
			while (m_bAtomRunning)
			{

				if (m_bMinimized) // minimized --> wait for next message --> reduces CPU load
				{
					GetMessageW(&msg, NULL, 0, 0);
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}

				// process full message queue before doing anything else
				while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
				{
					if (msg.message == WM_QUIT)
					{
						m_bAtomRunning = false;
						break;
					}

					TranslateMessage(&msg);
					DispatchMessageW(&msg);
				}


				// window should be closed --> wait for confirmation from thread (except on WM_QUIT)
				if (!m_bAtomRunning && msg.message != WM_QUIT)
				{
					// wait for thread to confirm m_bAtomRunning
					m_bAtomThreadConfirmRunning = true;
					while (m_bAtomThreadConfirmRunning);
				}
			}

			trd.join(); // wait for OpenGL thread to quit

			m_pInstance = nullptr;
			m_bRunning = false;
			m_hWnd = NULL;
		}

		UnregisterClassW(m_szWinClassName, NULL);

		m_pInstance = nullptr;

	}





	//----------------------------------------------------------------------------------------------
	// PROTECTED METHODS

	void OpenGLWin::minimize()
	{
		if (m_bMinimized)
			return;

		if (!m_bFullscreen)
			SendMessageW(m_hWnd, WM_SIZE, SIZE_MINIMIZED, MAKELPARAM(m_iWidth, m_iHeight));
		else
			ShowWindow(m_hWnd, SW_MINIMIZE);

		processMinimize();
	}

	void OpenGLWin::restore()
	{
		if (!m_bMinimized)
			return;

		processRestore();

		if (!m_bFullscreen)
			SendMessageW(m_hWnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(m_iWidth, m_iHeight));
		else
			ShowWindow(m_hWnd, SW_RESTORE);
	}

	void OpenGLWin::setFullscreen(HMONITOR monitor)
	{
		if (!m_bFullscreen)
		{
			RECT rect;
			GetWindowRect(m_hWnd, &rect);
			m_iWinPosX = rect.left;
			m_iWinPosY = rect.top;
		}

		// NULL --> primary monitor
		if (monitor == NULL)
		{
			const POINT pt = { 0, 0 };
			monitor = MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);
		}

		m_hMonitorFullscreen = monitor;
		m_bFullscreen = true;

		MONITORINFO mi = { sizeof(mi) };
		GetMonitorInfoW(monitor, &mi);
		DWORD dwStyle = GetWindowLongW(m_hWnd, GWL_STYLE);
		SetWindowLongW(m_hWnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);

		SetWindowPos(m_hWnd, HWND_TOPMOST, mi.rcMonitor.left, mi.rcMonitor.top,
			mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top,
			SWP_FRAMECHANGED);
	}

	void OpenGLWin::setWindowed()
	{
		if (!m_bFullscreen)
			return;

		m_bFullscreen = false;

		DWORD dwStyle = GetWindowLongW(m_hWnd, GWL_STYLE);
		DWORD dwExStyle = GetWindowLongW(m_hWnd, GWL_EXSTYLE);
		dwStyle &= ~WS_POPUP;
		dwStyle |= WS_OVERLAPPEDWINDOW;
		if (!m_bResizable)
			dwStyle &= ~(WS_MAXIMIZEBOX | WS_THICKFRAME);


		SetWindowLongW(m_hWnd, GWL_STYLE, dwStyle);
		RECT rect = { 0, 0, (LONG)m_iWinWidth, (LONG)m_iWinHeight };
		AdjustWindowRectEx(&rect, GetWindowLongW(m_hWnd, GWL_STYLE), FALSE, dwExStyle);

		SetWindowPos(m_hWnd, HWND_TOPMOST, m_iWinPosX, m_iWinPosY, rect.right - rect.left,
			rect.bottom - rect.top, SWP_FRAMECHANGED);
	}

	void OpenGLWin::setWindowedSize(uint32_t width, uint32_t height)
	{
		m_iWinWidth = width;
		m_iWinHeight = height;

		// currently in windowed mode --> apply
		if (!m_bFullscreen)
		{
			RECT rect = { 0, 0, (LONG)m_iWinWidth, (LONG)m_iWinHeight };
			AdjustWindowRectEx(&rect, GetWindowLongW(m_hWnd, GWL_STYLE), FALSE,
				GetWindowLongW(m_hWnd, GWL_EXSTYLE));

			SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, rect.right, rect.bottom, SWP_NOMOVE);
		}
	}

	void OpenGLWin::setTitle(const wchar_t* title) { SetWindowTextW(m_hWnd, title); }

	void OpenGLWin::setIcon(HICON BigIcon, HICON SmallIcon)
	{
		SendMessageW(m_hWnd, WM_SETICON, ICON_BIG, (LPARAM)BigIcon);
		SendMessageW(m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)SmallIcon);
	}

	OpenGLCoord OpenGLWin::getPixelCoord(int x, int y)
	{
		OpenGLCoord oResult;
		const OpenGLCoord oScreenCenter = { m_iCachedWidth / 2.0f, m_iCachedHeight / 2.0f };

		if (x >= oScreenCenter.x)
			oResult.x = (x - oScreenCenter.x) / oScreenCenter.x;
		else
			oResult.x = -1.0f + x / oScreenCenter.x;

		if (y >= oScreenCenter.y)
			oResult.y = -1.0f * (y - oScreenCenter.y) / oScreenCenter.y;
		else
			oResult.y = 1.0f - y / oScreenCenter.y;

		return oResult;
	}





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	void OpenGLWin::setMinMaxStruct(LPARAM lParam)
	{
		MINMAXINFO& mmi = *((MINMAXINFO*)lParam);
		DWORD dwStyle;
		if (m_hWnd != NULL)
			dwStyle = GetWindowLong(m_hWnd, GWL_STYLE);
		else
			dwStyle = m_dwStyleCache;

		// maximum, if set
		RECT rect = { 0, 0, (LONG)m_iWidthMax, (LONG)m_iHeightMax };
		AdjustWindowRect(&rect, dwStyle, FALSE);
		if (m_iWidthMax > 0)
			mmi.ptMaxTrackSize.x = rect.right - rect.left;
		if (m_iHeightMax > 0)
			mmi.ptMaxTrackSize.y = rect.bottom - rect.top;

		// minimum, if set
		rect = { 0, 0, (LONG)m_iWidthMin, (LONG)m_iHeightMin };
		AdjustWindowRect(&rect, dwStyle, FALSE);
		if (m_iWidthMin > 0)
			mmi.ptMinTrackSize.x = rect.right - rect.left;
		if (m_iHeightMin > 0)
			mmi.ptMinTrackSize.y = rect.bottom - rect.top;

	}

	void OpenGLWin::OpenGLThread(HDC hDC)
	{
		// start of OpenGL initialization

		PIXELFORMATDESCRIPTOR pfd =
		{
			sizeof(PIXELFORMATDESCRIPTOR), 1,
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
			PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			PFD_MAIN_PLANE, 0, 0, 0, 0
		};
		int pf = ChoosePixelFormat(hDC, &pfd);
		SetPixelFormat(hDC, pf, &pfd);

		HGLRC hGLRC = wglCreateContext(hDC);
		if (!wglMakeCurrent(hDC, hGLRC))
		{
			wglDeleteContext(hGLRC);
			m_bAtomRunning = false;
			m_bAtomThreadConfirmRunning = true;
			throw std::exception("Couldn't make the OpenGL instance the current one");
		}

		cacheSize();
		glViewport(0, 0, m_iCachedWidth, m_iCachedHeight); // initial size

		// get VSync function pointer
		wglSwapInterval_t* wglSwapInterval =
			(wglSwapInterval_t*)wglGetProcAddress("wglSwapIntervalEXT");

		bool bVSync = m_bAtomVSync;

		if (bVSync)
			wglSwapInterval(1);
		else
			wglSwapInterval(0);

		// end of OpenGL initialization


		m_bAtomRunning = OnCreate(); // set running value for main thread
		m_bAtomThreadConfirmRunning = false; // confirm answer to main thread

		if (m_bAtomRunning)
		{
			bool bRunning = true; // m_bAtomThreadConfirmRunning must be handled before quitting

			auto time1 = std::chrono::system_clock::now();
			auto time2 = time1;
			std::chrono::duration<float> oElapsed;

			while (bRunning)
			{
				if (m_bMinimized) // minimized-- > wait for next message-- > reduces CPU load
				{
					std::unique_lock<std::mutex> lm(m_muxMinimize);
					m_cvMinimize.wait(lm);

					time1 = std::chrono::system_clock::now(); // time frozen during minimized state
				}

				glClear(GL_COLOR_BUFFER_BIT); // clear buffer

				// always make sure the viewport is correct
				cacheSize();
				glViewport(0, 0, m_iCachedWidth, m_iCachedHeight);


				// calculate time
				time2 = std::chrono::system_clock::now();
				oElapsed = time2 - time1;
				time1 = time2;

				m_bAtomRunning = OnUpdate(oElapsed.count());

				SwapBuffers(hDC); // refresh display

				// handle VSync change refresh
				if (m_bAtomVSync != bVSync)
				{
					bVSync = m_bAtomVSync;
					if (bVSync)
						wglSwapInterval(1);
					else
						wglSwapInterval(0);
				}

				// if VSync is enabled, wait for full redraw
				if (bVSync)
					DwmFlush();

				// handle main thread's destruction request
				if (m_bAtomThreadConfirmRunning)
				{
					m_bAtomRunning = !OnDestroy();
					m_bAtomThreadConfirmRunning = false;
					bRunning = m_bAtomRunning;
				}
			}
		}

		wglDeleteContext(hGLRC);
	}

	void OpenGLWin::processMinimize()
	{
		m_bMinimized = true;
	}

	void OpenGLWin::processRestore()
	{
		std::unique_lock<std::mutex> lm(m_muxMinimize);
		m_cvMinimize.notify_all();
		m_bMinimized = false;
	}

	void OpenGLWin::cacheSize()
	{
		// transfer size data using variable in-between, since copy assignment of atomic variables
		// is not possible (function deleted)

		uint32_t iWidth = m_iWidth, iHeight = m_iHeight;
		m_iCachedWidth = iWidth;
		m_iCachedHeight = iHeight;
	}

}