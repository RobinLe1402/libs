#include "rl/lib/rlOpenGL_ST/Window.hpp"

#include "rl/lib/rlOpenGL_ST/Application.hpp"

namespace lib = rl::OpenGL_ST;



lib::Window::Window(const wchar_t* szClassName, const WindowStartupConfig& config) :
	m_sClassName(szClassName), m_iWidth(config.iWidth), m_iHeight(config.iHeight)
{
	auto hInstance = GetModuleHandle(NULL);

	m_oWC.lpszClassName = m_sClassName.c_str();
	m_oWC.hbrBackground = GetSysColorBrush(COLOR_WINDOW);
	m_oWC.hCursor = LoadCursor(NULL, IDC_ARROW);
	m_oWC.hInstance = hInstance;
	m_oWC.lpfnWndProc = &lib::Application::WindowProc;

	m_iAtom = RegisterClassW(&m_oWC);

	if (!m_iAtom)
		throw std::exception("Couldn't register window class.");

	// todo: save class name in static Application member variable

	DWORD dwStyle = WS_OVERLAPPEDWINDOW;
	if (config.bVisible)
		dwStyle |= WS_VISIBLE;

	m_hWnd = CreateWindowExW(0, m_sClassName.c_str(), L"Test Window", dwStyle,
		CW_USEDEFAULT, CW_USEDEFAULT, m_iWidth, m_iHeight, NULL, NULL, hInstance, this);
}

lib::Window::~Window()
{
	UnregisterClassW(m_sClassName.c_str(), m_oWC.hInstance);
}

void lib::Window::show()
{
	// ToDo: check window state, maybe maximize or minimize
	ShowWindow(m_hWnd, SW_SHOW);
}

void lib::Window::hide()
{
	// ToDo: save window state?
	ShowWindow(m_hWnd, SW_HIDE);
}

void lib::Window::close()
{
	CloseWindow(m_hWnd); // ToDo: DestroyWindow() instead?
}
