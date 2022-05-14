#include "rl/lib/rlOpenGL/Core.hpp"

namespace lib = rl::OpenGL;


// private
struct WindowStyleAndSizes
{
	DWORD dwStyle;
	unsigned iBorderWidth, iBorderHeight;
	unsigned iMinClientWidth, iMinClientHeight;
};

// private
WindowStyleAndSizes GetStyleAndSizes(bool bFullscreen, bool bResizable)
{
	WindowStyleAndSizes oResult = {};

	if (!bFullscreen)
	{
		oResult.dwStyle = WS_OVERLAPPEDWINDOW;

		if (!bResizable)
			oResult.dwStyle &= ~(WS_MAXIMIZEBOX | WS_THICKFRAME);

		RECT rect = { 0, 0, 0, 0 };
		AdjustWindowRect(&rect, oResult.dwStyle, false);
		oResult.iBorderWidth = rect.right - rect.left;
		oResult.iBorderHeight = rect.bottom - rect.top;
	}
	else
	{
		// iBorderWidth and iBorderHeight remain 0
		oResult.dwStyle = WS_POPUP;
	}

	oResult.iMinClientWidth = GetSystemMetrics(SM_CXMIN) - oResult.iBorderWidth;
	oResult.iMinClientHeight = GetSystemMetrics(SM_CYMIN) - oResult.iBorderHeight;

	return oResult;
}


lib::Window* lib::Window::s_pInstance = nullptr;



void lib::Window::GetOSMinWindowedSize(bool bResizable, unsigned& iX, unsigned& iY)
{
	const auto oData = GetStyleAndSizes(false, bResizable);
	iX = oData.iMinClientWidth;
	iY = oData.iMinClientHeight;
}

LRESULT WINAPI lib::Window::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static Window& o = *s_pInstance;
	static bool s_bFirstRestore = true;

	switch (uMsg)
	{
	case WM_SYSCOMMAND:
		switch (wParam)
		{
		case SC_MAXIMIZE:
			o.m_bMaximized = true;
			if (o.m_bMinimized)
			{
				o.m_bMinimized = false;
				o.m_fnOnRestore();
			}
			break;

		case SC_RESTORE:
			// if the window is minimized, "restored" means exiting the minimized state.
			if (o.m_bMinimized)
			{
				o.m_bMinimized = false;
				o.m_fnOnRestore();
			}
			// if the window is not minimized, "restored" means undoing the maximization.
			else
				o.m_bMaximized = false;
			break;

		case SC_MINIMIZE:
			o.m_bMinimized = true;
			o.m_fnOnMinimize();
			break;
		}
		return DefWindowProc(hWnd, uMsg, wParam, lParam);

	case WM_SETFOCUS:
	case WM_KILLFOCUS:
		o.m_fnOnMessage(uMsg, wParam, lParam);
		break;

	case WM_WINDOWPOSCHANGING:
	{
		auto& oWindowPos = *reinterpret_cast<WINDOWPOS*>(lParam);
		if ((oWindowPos.flags & SWP_NOSIZE) == 0)
			o.m_fnOnMessage(uMsg, wParam, lParam);
		break;
	}

	case WM_WINDOWPOSCHANGED:
	{
		auto& oWindowPos = *reinterpret_cast<const WINDOWPOS*>(lParam);
		o.m_iWidth = oWindowPos.cx - o.m_iBorderWidth;
		o.m_iHeight = oWindowPos.cy - o.m_iBorderHeight;

		if ((oWindowPos.flags & SWP_NOSIZE) == 0)
			o.m_fnOnMessage(uMsg, wParam, lParam);
		break; // don't call DefWindowProc --> no WM_SIZE and WM_MOVE
	}

	case WM_GETMINMAXINFO:
		if (!o.m_bFullscreen)
		{
			MINMAXINFO& mmi = *reinterpret_cast<MINMAXINFO*>(lParam);

			// maximum
			if (o.m_iMaxWidth > 0 || o.m_iMaxHeight > 0)
			{
				if (o.m_iMaxWidth > 0)
					mmi.ptMaxTrackSize.x = o.m_iMaxWidth + o.m_iBorderWidth;
				if (o.m_iMaxHeight > 0)
					mmi.ptMaxTrackSize.y = o.m_iMaxHeight + o.m_iBorderHeight;
			}

			// minimum
			if (o.m_iMinWidth > 0 || o.m_iMinHeight > 0)
			{
				if (o.m_iMinWidth > 0)
					mmi.ptMinTrackSize.x = o.m_iMinWidth + o.m_iBorderWidth;
				if (o.m_iMinHeight > 0)
					mmi.ptMinTrackSize.y = o.m_iMinHeight + o.m_iBorderHeight;
			}
		}

		break;



	case WM_CLOSE:
		o.m_bCloseRequested = true;
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

bool lib::Window::create(const WindowConfig& cfg, MessageCallback fnOnMessage,
	VoidCallback fnOnMinimize, VoidCallback fnOnRestore)
{
	destroy();

	s_pInstance = this;
	m_trdidApplication = std::this_thread::get_id();

	if (cfg.iWidth == 0 || cfg.iHeight == 0)
		return false;

	if (cfg.iWidth > INT_MAX || cfg.iHeight > INT_MAX ||
		cfg.iMinWidth > INT_MAX || cfg.iMinHeight > INT_MAX ||
		cfg.iMaxWidth > INT_MAX || cfg.iMaxHeight > INT_MAX)
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
	m_fnOnMinimize = fnOnMinimize;
	m_fnOnRestore = fnOnRestore;

	std::unique_lock lm(m_muxState);
	m_trdMessageLoop = std::thread(&lib::Window::threadFunction, this, cfg);
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

	std::unique_lock lm(m_muxState);
	PostMessage(m_hWnd, WM_DESTROY, 0, 0);
	m_cvState.wait(lm);

	if (m_trdMessageLoop.joinable())
		m_trdMessageLoop.join();

	clear();
}

void lib::Window::show()
{
	invoke(ShowWindow, m_hWnd, SW_SHOW);
	invoke(SetForegroundWindow, m_hWnd);
}

void lib::Window::setTitle(const wchar_t* szTitle)
{
	if (!m_bMessageLoop)
		return;

	m_sTitle = szTitle;
	invoke(SetWindowTextW, m_hWnd, m_sTitle.c_str());
}

void lib::Window::setTitle(const char* szTitle)
{
	if (!m_bMessageLoop)
		return;


	const size_t len = strlen(szTitle);

	m_sTitle.clear();
	m_sTitle.reserve(len);

	while (szTitle[0] != 0)
	{
		if (szTitle[0] & 0x80)
			m_sTitle += L'?';
		else
			m_sTitle += szTitle[0];

		++szTitle;
	}

	invoke(SetWindowTextW, m_hWnd, m_sTitle.c_str());
}

void lib::Window::minimize()
{
	if (!m_bMessageLoop || m_bMinimized)
		return;

	if (!m_bFullscreen)
		invoke(SendMessage, m_hWnd, WM_SIZE, SIZE_MINIMIZED, MAKELPARAM(m_iWidth, m_iHeight));
	else
		invoke(ShowWindow, m_hWnd, SW_MINIMIZE);
}

void lib::Window::setSize(unsigned iWidth, unsigned iHeight)
{
	if (!m_bMessageLoop || m_bFullscreen)
		return;

	invoke(SendMessage, m_hWnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(iWidth, iHeight));
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

template <typename TResult, typename... TArgsFn, typename... TArgsCall>
void lib::Window::invoke(TResult(*fn)(TArgsFn...), TArgsCall... args)
{
#pragma warning(disable : 4834) // discarding return value of function with 'nodiscard' attribute

	if (m_trdidApplication == std::this_thread::get_id())
	{
		std::thread trdTMP(fn, args...);
		trdTMP.detach();
	}
	else
		fn(args...);

#pragma warning(default : 4834)
}

void lib::Window::threadFunction(WindowConfig cfg)
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



		// handle the minimum size by the OS

		unsigned iOSMinX = 0, iOSMinY = 0;
		Window::GetOSMinWindowedSize(m_bResizable, iOSMinX, iOSMinY);

		if (!m_bFullscreen)
		{
			if (m_iWidth < iOSMinX)
				m_iWidth = iOSMinX;
			if (m_iHeight < iOSMinY)
				m_iHeight = iOSMinY;
		}

		if (m_iMaxWidth > 0 && m_iMaxWidth < iOSMinX)
			m_iMaxWidth = iOSMinX;
		if (m_iMaxHeight > 0 && m_iMaxHeight < iOSMinY)
			m_iMaxHeight = iOSMinY;



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

		// calculate the window width
		unsigned iNativeWidth = m_iWidth;
		unsigned iNativeHeight = m_iHeight;
		if (!m_bFullscreen)
		{
			iNativeWidth += m_iBorderWidth;
			iNativeHeight += m_iBorderHeight;
		}



		// calculate the window position

		int iPosX, iPosY;
		MONITORINFO mi = { sizeof(mi) };
		GetMonitorInfo(m_hMonitorFullscreen, &mi);

		// default windowed position
		m_iWindowX =
			mi.rcWork.left + (int(mi.rcWork.right - mi.rcWork.left) / 2 - (int)iNativeWidth / 2);
		m_iWindowY =
			mi.rcWork.top + (int(mi.rcWork.bottom - mi.rcWork.top) / 2 - (int)iNativeHeight / 2);

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



		m_hWnd = CreateWindowW(m_sClassName.c_str(), cfg.sTitle.c_str(), dwStyle, iPosX, iPosY,
			iNativeWidth, iNativeHeight, NULL, NULL, NULL, NULL);

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
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) && msg.message != WM_QUIT)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
			break;

		if (m_bMinimized) // minimized --> wait for next message --> reduces CPU load
		{
			GetMessage(&msg, NULL, 0, 0);
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
				break;
		}
	}
	m_bMessageLoop = false;

	OnDestroy();

	std::unique_lock lm(m_muxState);
	m_bThreadRunning = false;
	m_cvState.notify_one();
}

DWORD lib::Window::refreshStyle()
{
	const auto oData = GetStyleAndSizes(m_bFullscreen, m_bResizable);
	const DWORD dwNewStyle = oData.dwStyle | (GetWindowLong(m_hWnd, GWL_STYLE) & WS_VISIBLE);

	m_iBorderWidth = oData.iBorderWidth;
	m_iBorderHeight = oData.iBorderHeight;
	m_iOSMinWidth = oData.iMinClientWidth;
	m_iOSMinHeight = oData.iMinClientHeight;

	return dwNewStyle;
}

void lib::Window::clear()
{
	m_hWnd = NULL;
	m_bMessageLoop = false;
	m_bThreadRunning = false;

	m_fnOnMessage = nullptr;
	m_fnOnMinimize = nullptr;
	m_fnOnRestore = nullptr;

	m_iWidth = m_iHeight = 0;
	m_iBorderWidth = m_iBorderHeight = 0;
	m_iOSMinWidth = m_iOSMinHeight = 0;
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
	m_bCloseRequested = false;
}
