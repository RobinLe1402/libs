#include "rl/lib/rlOpenGL/Core.hpp"

namespace lib = rl::OpenGL;

// ToDo: Fix screen flickering on gaining/losing focus when in fullscreen mode



lib::Window* lib::Window::s_pInstance = nullptr;

LRESULT WINAPI lib::Window::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static Window& o = *s_pInstance;
	static bool s_bFirstRestore = true;

	switch (uMsg)
	{
	case WM_SETFOCUS:
	case WM_KILLFOCUS:
		o.m_fnOnMessage(uMsg, wParam, lParam);
		break;

	case WM_SIZING:
		o.m_fnOnMessage(uMsg, wParam, lParam);
		{
			auto& rect = *reinterpret_cast<RECT*>(lParam);
			o.m_iNativeWidth = rect.right - rect.left;
			o.m_iNativeHeight = rect.bottom - rect.top;

			o.m_iWidth = o.m_iNativeWidth - o.m_iClientToScreenX;
			o.m_iHeight = o.m_iNativeHeight - o.m_iClientToScreenY;
		}
		break;

	case WM_SIZE:

		if (wParam != SIZE_MINIMIZED)
		{
			o.m_iWidth = LOWORD(lParam);
			o.m_iHeight = HIWORD(lParam);

			o.m_iNativeWidth = o.m_iWidth + o.m_iClientToScreenX;
			o.m_iNativeHeight = o.m_iHeight + o.m_iClientToScreenY;
		}

		o.m_bMinimized = (wParam == SIZE_MINIMIZED);
		if (!o.m_bFullscreen)
			o.m_bMaximized = (wParam == SIZE_MAXIMIZED);

		o.m_fnOnMessage(uMsg, wParam, lParam);
		break;

	case WM_GETMINMAXINFO:
		if (!o.m_bFullscreen)
		{
			MINMAXINFO& mmi = *reinterpret_cast<MINMAXINFO*>(lParam);

			// maximum
			if (o.m_iMaxWidth > 0 || o.m_iMaxHeight > 0)
			{
				if (o.m_iMaxWidth > 0)
					mmi.ptMaxTrackSize.x = o.m_iMaxWidth + o.m_iClientToScreenX;
				if (o.m_iMaxHeight > 0)
					mmi.ptMaxTrackSize.y = o.m_iMaxHeight + o.m_iClientToScreenY;
			}

			// minimum
			if (o.m_iMinWidth > 0 || o.m_iMinHeight > 0)
			{
				if (o.m_iMinWidth > 0)
					mmi.ptMinTrackSize.x = o.m_iMinWidth + o.m_iClientToScreenX;
				if (o.m_iMinHeight > 0)
					mmi.ptMinTrackSize.y = o.m_iMinHeight + o.m_iClientToScreenY;
			}
		}

		break;



	case WM_CLOSE:
		if (!o.m_bAppClose)
		{
			o.m_bWinClose = true;
			if (!o.m_fnOnClose())
				break;
		}
		DestroyWindow(hWnd);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_QUIT:
		break;

	default:
		if (o.OnMessage(uMsg, wParam, lParam))
			return 0;

		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

lib::Window::Window(const wchar_t* szClassName) : m_sClassName(szClassName) { }

lib::Window::~Window() { destroy(); }

bool lib::Window::create(const WindowConfig& cfg,
	MessageCallback fnOnMessage, CloseCallback fnOnClose)
{
	destroy();

	s_pInstance = this;

	if (cfg.iWidth == 0 || cfg.iHeight == 0)
		return false;

	if (cfg.iMinWidth > 0)
	{
		if (cfg.iWidth < cfg.iMinWidth ||
			(cfg.iMaxWidth > 0 && cfg.iMaxWidth < cfg.iMinWidth))
			return false; // invalid min width
	}
	if (cfg.iMinHeight > 0)
	{
		if (cfg.iHeight < cfg.iMinHeight ||
			(cfg.iMaxHeight > 0 && cfg.iMaxHeight < cfg.iMinHeight))
			return false; // invalid min height
	}
	if (cfg.iMaxWidth && (cfg.iWidth > cfg.iMaxWidth))
		return false; // invalid max width
	if (cfg.iMaxHeight && (cfg.iHeight > cfg.iMaxHeight))
		return false; // invalid max height


	m_fnOnMessage = fnOnMessage;
	m_fnOnClose = fnOnClose;

	std::unique_lock lm(m_muxState);
	m_trdMessageLoop = std::thread(&lib::Window::messageLoop, this, cfg);
	m_cvState.wait(lm);
	if (!m_bMessageLoop && m_trdMessageLoop.joinable())
		m_trdMessageLoop.join();
	else
		m_bThreadRunning = true;

	return m_bMessageLoop;
}

void lib::Window::destroy()
{
	if (!m_bThreadRunning)
		return;

	if (!m_bWinClose)
	{
		std::unique_lock lm(m_muxState);
		m_bAppClose = true;
		SendMessage(m_hWnd, WM_CLOSE, 0, 0); // CloseWindow() doesn't always work here
		m_cvState.wait(lm);
	}

	if (m_trdMessageLoop.joinable())
		m_trdMessageLoop.join();

	clear();
	m_bThreadRunning = false;
}

void lib::Window::show()
{
	ShowWindow(m_hWnd, SW_SHOW);
	SetForegroundWindow(m_hWnd);
}

void lib::Window::setTitle(const wchar_t* szTitle)
{
	if (!m_bMessageLoop)
		return;

	m_sTitle = szTitle;
	SetWindowTextW(m_hWnd, szTitle);
}

void lib::Window::minimize()
{
	if (!m_bMessageLoop || m_bMinimized)
		return;

	if (!m_bFullscreen)
		SendMessage(m_hWnd, WM_SIZE, SIZE_MINIMIZED, MAKELPARAM(m_iWidth, m_iHeight));
	else
		ShowWindow(m_hWnd, SW_MINIMIZE);
}

void lib::Window::setSize(unsigned iWidth, unsigned iHeight)
{
	if (!m_bMessageLoop || m_bFullscreen)
		return;

	SendMessage(m_hWnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(iWidth, iHeight));
}

void lib::Window::setFullscreen(bool bFullscreen, HMONITOR hMon)
{
	if (!running())
		return;

	if (hMon == NULL)
		hMon = m_hMonitorFullscreen;

	const bool bFullscreenChange =
		!(bFullscreen && m_bFullscreen) || (!bFullscreen && !m_bFullscreen);

	if (!bFullscreenChange && (!bFullscreen || m_hMonitorFullscreen == hMon))
		return; // no change


	m_bFullscreen = bFullscreen;
	m_hMonitorFullscreen = hMon;

	if (bFullscreenChange) // fullscreen mode was toggled
	{
		WINDOWPLACEMENT wndpl = { sizeof(WINDOWPLACEMENT) };
		GetWindowPlacement(m_hWnd, &wndpl);

		const DWORD dwStyle = refreshStyle();
		if (bFullscreen) // entering fullscreen --> save the current window data
		{
			m_iRestoredWidth = wndpl.rcNormalPosition.right - wndpl.rcNormalPosition.left;
			m_iRestoredHeight = wndpl.rcNormalPosition.bottom - wndpl.rcNormalPosition.top;
			m_iWindowX = wndpl.rcNormalPosition.left;
			m_iWindowY = wndpl.rcNormalPosition.top;
			m_iMaximizedX = wndpl.ptMaxPosition.x;
			m_iMaximizedY = wndpl.ptMaxPosition.y;

			SetWindowLong(m_hWnd, GWL_STYLE, dwStyle);
		}
		else // exiting fullscreen --> set window data
		{
			if (m_bMaximized)
				wndpl.showCmd = SW_SHOWMAXIMIZED;
			else
				wndpl.showCmd = SW_SHOWNORMAL;
			wndpl.rcNormalPosition.left = m_iWindowX;
			wndpl.rcNormalPosition.top = m_iWindowY;
			wndpl.rcNormalPosition.right = m_iWindowX + m_iRestoredWidth;
			wndpl.rcNormalPosition.bottom = m_iWindowY + m_iRestoredHeight;
			wndpl.ptMaxPosition = { m_iMaximizedX, m_iMaximizedY };

			SetWindowLong(m_hWnd, GWL_STYLE, dwStyle);
			SetWindowPlacement(m_hWnd, &wndpl);
		}
	}

	if (bFullscreen)
	{
		MONITORINFO mi = { sizeof(mi) };
		GetMonitorInfoW(hMon, &mi);

		SetWindowPos(m_hWnd, HWND_TOPMOST, mi.rcMonitor.left, mi.rcMonitor.top,
			mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top,
			SWP_FRAMECHANGED);
	}
}

void lib::Window::messageLoop(WindowConfig cfg)
{
	try
	{
		std::unique_lock lm(m_muxState); // wait until main thread is waiting for notification

		m_sTitle = cfg.sTitle;
		m_bFullscreen = cfg.bFullscreen;
		m_bResizable = cfg.bResizable;
		m_iRestoredWidth = cfg.iWidth;
		m_iRestoredHeight = cfg.iHeight;
		m_iMinWidth = cfg.iMinWidth;
		m_iMinHeight = cfg.iMinHeight;
		m_iMaxWidth = cfg.iMaxWidth;
		m_iMaxHeight = cfg.iMaxHeight;
		m_hMonitorFullscreen = cfg.hMonintorFullscreen;

		if (m_hMonitorFullscreen == NULL)
		{
			const POINT pt = { 0, 0 };
			m_hMonitorFullscreen = MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);
		}
		if (m_bFullscreen)
		{
			MONITORINFO mi = { sizeof(mi) };
			GetMonitorInfo(m_hMonitorFullscreen, &mi);
			m_iWidth = mi.rcMonitor.right - mi.rcMonitor.left;
			m_iHeight = mi.rcMonitor.bottom - mi.rcMonitor.top;
		}
		else
		{
			m_iWidth = cfg.iWidth;
			m_iHeight = cfg.iHeight;
		}



		WNDCLASSEXW wc = {};
		wc.cbSize = sizeof(wc);
		wc.hInstance = GetModuleHandle(NULL);
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wc.lpfnWndProc = WindowProc;
		wc.lpszClassName = m_sClassName.c_str();
		wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
		wc.hIcon = cfg.hIconBig;
		wc.hIconSm = cfg.hIconSmall;

		if (!RegisterClassExW(&wc))
			throw std::exception();


		const DWORD dwStyle = refreshStyle();



		// calculate the window position

		int iPosX, iPosY;
		MONITORINFO mi = { sizeof(mi) };
		GetMonitorInfo(m_hMonitorFullscreen, &mi);

		// default windowed position
		m_iWindowX = mi.rcWork.left + ((mi.rcWork.right - mi.rcWork.left - m_iWidth) / 2);
		m_iWindowY = mi.rcWork.top + ((mi.rcWork.bottom - mi.rcWork.top - m_iHeight) / 2);

		if (m_bFullscreen) // fullscreen --> top left
		{
			iPosX = mi.rcMonitor.left;
			iPosY = mi.rcMonitor.top;
		}
		else // windowed --> screen center
		{
			iPosX = m_iWindowX;
			iPosY = m_iWindowY;
		}



		// calculate the window width
		m_iNativeWidth = m_iWidth;
		m_iNativeHeight = m_iHeight;
		if (!m_bFullscreen)
		{
			m_iNativeWidth += m_iClientToScreenX;
			m_iNativeHeight += m_iClientToScreenY;
		}



		m_hWnd = CreateWindowW(m_sClassName.c_str(), cfg.sTitle.c_str(), dwStyle, iPosX, iPosY,
			m_iNativeWidth, m_iNativeHeight, NULL, NULL, NULL, NULL);

		if (m_hWnd == NULL)
			throw std::exception();

	}
	catch (...)
	{
		clear();
		m_cvState.notify_one(); // notify main thread that create() has failed
		return;
	}

	m_bMessageLoop = true;
	m_cvState.notify_one(); // notify main thread that create() has succeeded

	OnCreate();

	MSG msg{};
	while (true)
	{
		// process full message queue before doing anything else
		while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE) && msg.message != WM_QUIT)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
			break;

		if (m_bMinimized) // minimized --> wait for next message --> reduces CPU load
		{
			GetMessageW(&msg, NULL, 0, 0);
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
				break;
		}
	}
	m_bMessageLoop = false;

	OnDestroy();

	std::unique_lock lm(m_muxState);
	m_cvState.notify_one();
}

void lib::Window::clear()
{
	m_hWnd = NULL;
	m_bAppClose = m_bWinClose = false;
	m_bMessageLoop = false;
	m_bThreadRunning = false;

	m_fnOnMessage = nullptr;
	m_fnOnClose = nullptr;

	m_iWidth = m_iHeight = 0;
	m_iNativeWidth = m_iNativeHeight = 0;
	m_iClientToScreenX = m_iClientToScreenY = 0;
	m_iMinWidth = m_iMinHeight = 0;
	m_iMaxWidth = m_iMaxHeight = 0;
	m_iRestoredWidth = m_iRestoredHeight = 0;

	m_iWindowX = m_iWindowY = 0;
	m_iMaximizedX = m_iMaximizedY = 0;

	m_sTitle.clear();
	m_hMonitorFullscreen = NULL;
	m_bResizable = false;
	m_bMinimized = false;
	m_bMaximized = false;
	m_bFullscreen = false;
}

DWORD lib::Window::refreshStyle()
{
	DWORD dwOldStyle = GetWindowLong(m_hWnd, GWL_STYLE);
	DWORD dwNewStyle = 0;
	if (dwOldStyle & WS_VISIBLE)
		dwNewStyle = WS_VISIBLE;

	if (!m_bFullscreen)
	{
		dwNewStyle |= WS_OVERLAPPEDWINDOW;

		if (!m_bResizable)
			dwNewStyle &= ~(WS_MAXIMIZEBOX | WS_THICKFRAME);

		RECT rect = { 0, 0, 0, 0 };
		AdjustWindowRect(&rect, dwNewStyle, false);
		m_iClientToScreenX = rect.right - rect.left;
		m_iClientToScreenY = rect.bottom - rect.top;
	}
	else
	{
		m_iClientToScreenX = m_iClientToScreenY = 0;
		dwNewStyle |= WS_POPUP;
	}

	return dwNewStyle;
}
