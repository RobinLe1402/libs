#include "rl/lib/OpenGLWin.hpp"

namespace lib = rl::OpenGLWin;



lib::Window* lib::Window::s_pInstance = nullptr;

LRESULT WINAPI lib::Window::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static Window& o = *s_pInstance;

	switch (uMsg)
	{
	case WM_SIZING:
		o.m_oApplication.winMessage(uMsg, wParam, lParam);
		break;

	case WM_SIZE:

		if (wParam != SIZE_MINIMIZED)
		{
			o.m_iWidth = LOWORD(lParam);
			o.m_iHeight = HIWORD(lParam);

			RECT rect{};
			GetWindowRect(hWnd, &rect);
			o.m_iNativeWidth = rect.right - rect.left;
			o.m_iNativeHeight = rect.bottom - rect.top;
		}

		switch (wParam)
		{
		case SIZE_MINIMIZED:
			o.m_bMinimized = true;
			[[fallthrough]];
		case SIZE_MAXIMIZED:
		case SIZE_RESTORED:
			o.m_oApplication.winMessage(uMsg, wParam, lParam);
		}
		break;

	case WM_GETMINMAXINFO:
		if (!o.m_bFullscreen)
		{
			MINMAXINFO& mmi = *reinterpret_cast<MINMAXINFO*>(lParam);
			const DWORD dwStyle = GetWindowLong(hWnd, GWL_STYLE);


			RECT rect;

			// maximum
			if (o.m_iMaxWidth > 0 || o.m_iMaxHeight > 0)
			{
				rect = { 0, 0, (LONG)o.m_iMaxWidth, (LONG)o.m_iMaxHeight };
				AdjustWindowRect(&rect, dwStyle, FALSE);
				if (o.m_iMaxWidth > 0)
					mmi.ptMaxTrackSize.x = rect.right - rect.left;
				if (o.m_iMaxHeight > 0)
					mmi.ptMaxTrackSize.y = rect.bottom - rect.top;
			}

			// minimum
			if (o.m_iMinWidth > 0 || o.m_iMinHeight > 0)
			{
				rect = { 0, 0, (LONG)o.m_iMinWidth, (LONG)o.m_iMinHeight };
				AdjustWindowRect(&rect, dwStyle, FALSE);
				if (o.m_iMinWidth > 0)
					mmi.ptMinTrackSize.x = rect.right - rect.left;
				if (o.m_iMinHeight > 0)
					mmi.ptMinTrackSize.y = rect.bottom - rect.top;
			}
		}

		break;



	case WM_CLOSE:
		if (!o.m_bAppClose)
		{
			o.m_bWinClose = true;
			if (!o.m_oApplication.winClose())
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

lib::Window::Window(IApplication& oApplication) : m_oApplication(oApplication) { }

lib::Window::~Window() { destroy(); }

bool lib::Window::create(const WindowConfig& cfg)
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

bool lib::Window::windowToClient(unsigned& iWidth, unsigned& iHeight)
{
	if (m_iNativeWidth < m_iWidth || m_iNativeHeight < m_iHeight)
		return false;

	iWidth -= m_iNativeWidth - m_iWidth;
	iHeight -= m_iNativeHeight - m_iHeight;

	return true;
}

void lib::Window::messageLoop(WindowConfig cfg)
{
	try
	{
		std::unique_lock lm(m_muxState); // wait until main thread is waiting for notification

		m_sTitle = cfg.sTitle;
		m_bFullscreen = cfg.bFullscreen;
		m_bResizable = cfg.bResizable;
		m_iWindowedWidth = cfg.iWidth;
		m_iWindowedHeight = cfg.iHeight;
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
		wc.lpszClassName = s_szCLASSNAME;
		wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
		wc.hIcon = cfg.hIconBig;
		wc.hIconSm = cfg.hIconSmall;

		if (!RegisterClassExW(&wc))
			throw std::exception();

		DWORD dwStyle = 0;
		if (!m_bFullscreen)
		{
			dwStyle |= WS_OVERLAPPEDWINDOW;

			if (!m_bResizable)
				dwStyle &= ~(WS_MAXIMIZEBOX | WS_THICKFRAME);
		}
		else
			dwStyle |= WS_POPUP;



		// calculate the window position
		int iPosX, iPosY;
		if (m_bFullscreen) // fullscreen --> top left
		{
			iPosX = 0;
			iPosY = 0;
		}
		else // windowed --> screen center
		{
			MONITORINFO mi = { sizeof(mi) };
			GetMonitorInfo(m_hMonitorFullscreen, &mi);

			const int iCenterX = (mi.rcWork.right - mi.rcWork.left) / 2;
			const int iCenterY = (mi.rcWork.bottom - mi.rcWork.top) / 2;

			RECT rect = { 0, 0, (LONG)m_iWidth, (LONG)m_iHeight };
			AdjustWindowRect(&rect, dwStyle, FALSE);

			iPosX = mi.rcWork.left + iCenterX - (rect.right - rect.left) / 2;
			iPosY = mi.rcWork.top + iCenterY - (rect.bottom - rect.top) / 2;
		}



		// calculate the window width
		m_iNativeWidth = m_iWidth;
		m_iNativeHeight = m_iHeight;
		if (!m_bFullscreen)
		{
			RECT rect = { 0, 0, (LONG)m_iNativeWidth, (LONG)m_iNativeHeight };
			AdjustWindowRect(&rect, dwStyle, FALSE);
			m_iNativeWidth = rect.right - rect.left;
			m_iNativeHeight = rect.bottom - rect.top;
		}



		m_hWnd = CreateWindowW(s_szCLASSNAME, cfg.sTitle.c_str(), dwStyle, iPosX, iPosY,
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

	std::unique_lock lm(m_muxState);
	m_cvState.notify_one();
}

void lib::Window::clear()
{
	m_hWnd = NULL;
	m_sTitle.clear();
	m_bMessageLoop = false;
	m_bMinimized = false;
	m_bFullscreen = false;
	m_bResizable = false;
	m_hMonitorFullscreen = NULL;
	m_iWidth = 0;
	m_iHeight = 0;
	m_iWindowedWidth = 0;
	m_iWindowedHeight = 0;
}
