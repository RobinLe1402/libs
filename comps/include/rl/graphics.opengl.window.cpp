#include "graphics.opengl.window.hpp"

#include <stdint.h>
#include <thread>
#include <Windows.h>
#include <chrono>

// OpenGL
#include <GL/gl.h>
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "OpenGL32.lib")

// DWM
#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")

// RobinLe
#define ROBINLE_OPENGL_FUNCTIONS
#include "graphics.opengl.types.hpp"



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

		if (!m_pInstance)
			return 0;

		bool bProcessed = true;
		switch (uMsg)
		{

			//--------------------------------------------------------------------------------------
			// WINDOW/SYSTEM MENU

		case WM_SYSCOMMAND:
			if (wParam == m_pInstance->m_iMenuAbout)
				m_pInstance->OnAbout();
			else
				bProcessed = false;
			break;



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
			m_pInstance->OnLoseFocus();
			if (m_pInstance->m_bFullscreen)
				m_pInstance->minimize();
			break;

		case WM_SETFOCUS: // restore window when receiving focus in fullscreen mode
			m_pInstance->OnGainFocus();
			if (m_pInstance->m_bFullscreen)
				m_pInstance->restore();
			break;



			//--------------------------------------------------------------------------------------
			// CLOSING

		case WM_CLOSE:

			m_pInstance->wakeUpFromMinimized(); // wake up thread (if minimized)

			if (!m_pInstance->getQuitPermission()) // thread says "keep running" --> cancel closing
			{
				m_pInstance->restore(); // restore graphically
				return 0;
			}

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

		int iPosX, iPosY;
		if (m_bFullscreen)
		{
			iPosX = 0;
			iPosY = 0;
		}
		else // windowed --> screen center
		{
			MONITORINFO mi = { sizeof(mi) };
			GetMonitorInfoW(m_hMonitorFullscreen, &mi);

			const int iCenterX = (mi.rcWork.right - mi.rcWork.left) / 2;
			const int iCenterY = (mi.rcWork.bottom - mi.rcWork.top) / 2;

			RECT rect = { 0, 0, (LONG)m_iWidth, (LONG)m_iHeight };
			AdjustWindowRect(&rect, dwStyle, FALSE);

			iPosX = mi.rcWork.left + iCenterX - (rect.right - rect.left) / 2;
			iPosY = mi.rcWork.top + iCenterY - (rect.bottom - rect.top) / 2;
		}

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


		m_hWnd = CreateWindowW(m_szWinClassName, config.szInitialCaption, dwStyle,
			iPosX, iPosY, iWidth, iHeight, NULL, NULL, NULL, NULL);
		m_dwStyleCache = 0;

		if (config.bSysMenuAbout)
		{
			HMENU hMenu = GetSystemMenu(m_hWnd, false);
			InsertMenuW(hMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 255, NULL);
			InsertMenuW(hMenu, 0, MF_BYPOSITION, m_iMenuAbout, L"About");
		}

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

				// process full message queue before doing anything else
				while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessageW(&msg);
				}

				if (m_bMinimized) // minimized --> wait for next message --> reduces CPU load
				{
					GetMessageW(&msg, NULL, 0, 0);
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}

				if (msg.message == WM_QUIT)
				{
					m_bAtomRunning = false;
				}


				// window should be closed --> wait for confirmation from thread (except on WM_QUIT)
				if (!m_bAtomRunning && msg.message != WM_QUIT)
				{
					getQuitPermission();
				}
			}

			trd.join(); // wait for OpenGL thread to quit

			m_pInstance = nullptr;
			m_bRunning = false;
			m_hWnd = NULL;
		}
		else if (trd.joinable())
			trd.join();

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
		return OpenGL::GetPixelCoord(m_iCachedWidth, m_iCachedHeight, x, y);
	}

	OpenGLRect OpenGLWin::getPixelRect_Pos(int left, int top, int right, int bottom)
	{
		return OpenGL::GetPixelRect_Pos(m_iCachedWidth, m_iCachedHeight, left, top, right, bottom);
	}

	OpenGLRect OpenGLWin::getPixelRect(int left, int top, int width, int height)
	{
		return OpenGL::GetPixelRect(m_iCachedWidth, m_iCachedHeight, left, top, width, height);
	}

	void OpenGLWin::getVersion(uint8_t(&dest)[4])
	{
		static uint8_t version[4] = { 0, 5, 0, 0 };

		memcpy_s(dest, sizeof(dest), version, sizeof(version));
	}


	void OpenGLWin::OnAbout()
	{
		char szMsg[32]{};
		strcat_s(szMsg, "RobinLe OpenGL Engine V");

		uint8_t version[4];
		getVersion(version);

		char szInt[4]{};



		_itoa_s(version[0], szInt, 10);
		strcat_s(szMsg, szInt);
		memset(szInt, 0, 3);

		strcat_s(szMsg, ".");

		_itoa_s(version[1], szInt, 10);
		strcat_s(szMsg, szInt);
		memset(szInt, 0, 3);

		strcat_s(szMsg, ".");

		_itoa_s(version[2], szInt, 10);
		strcat_s(szMsg, szInt);
		memset(szInt, 0, 3);

		strcat_s(szMsg, ".");

		_itoa_s(version[3], szInt, 10);
		strcat_s(szMsg, szInt);
		memset(szInt, 0, 3);



		MessageBoxA(m_hWnd, szMsg, "About", MB_OK);
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
		m_bAtomOpenGLThreadRunning = true;
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
			SetForegroundWindow(m_hWnd); // window should be active after creation

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
					m_bAtomOpenGLThreadRunning = false;
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
		m_bAtomIdle = true;
	}

	void OpenGLWin::processRestore()
	{
		if (m_bAtomIdle)
			wakeUpFromMinimized();

		m_bMinimized = false;
	}

	void OpenGLWin::wakeUpFromMinimized()
	{
		std::unique_lock<std::mutex> lm(m_muxMinimize);
		m_cvMinimize.notify_all();
		m_bAtomIdle = false;
	}

	void OpenGLWin::cacheSize()
	{
		// transfer size data using variable in-between, since copy assignment of atomic variables
		// is not possible (function deleted)

		uint32_t iWidth = m_iWidth, iHeight = m_iHeight;
		m_iCachedWidth = iWidth;
		m_iCachedHeight = iHeight;
	}

	bool OpenGLWin::getQuitPermission()
	{
		if (!m_bAtomOpenGLThreadRunning)
			return true;

		m_bAtomThreadConfirmRunning = true;
		while (m_bAtomThreadConfirmRunning); // wait for thread to answer
		return !m_bAtomRunning;
	}

}