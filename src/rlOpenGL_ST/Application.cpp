#include "rl/lib/rlOpenGL_ST/Application.hpp"
#include "rl/lib/rlOpenGL_ST/Window.hpp"

namespace lib = rl::OpenGL_ST;


lib::Application lib::Application::s_oInstance;
lib::Application &lib::ThisApplication = Application::GetInstance();


void lib::Application::processMessages()
{
	MSG msg{};
	while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

lib::Window* lib::Application::getMainWindow()
{
	if (m_oWindows.size() == 0)
		return nullptr;
	else
		return m_oWindows[0];
}

void lib::Application::run()
{
	if (m_oWindows.size() > 0)
		m_oWindows[0]->show();

	MSG msg{};
	while (true)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);

			if (msg.message == WM_QUIT)
				break;
		}

		if (msg.message == WM_QUIT)
			break;

		// todo: application handling?
	}
}

LRESULT lib::Application::processMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	bool bDefProc = true;

	Window* pWindow = nullptr;
	for (auto p : m_oWindows)
	{
		if (p->getHandle() == hWnd)
		{
			pWindow = p;
			break;
		}
	}

	switch (uMsg)
	{
	case WM_CLOSE:
		if (pWindow) // known window
			pWindow->close();
		else // unknown window
			DestroyWindow(hWnd);
		break;

	case WM_DESTROY:
		if (!pWindow || m_oWindows[0] == pWindow) // unknown/main window
			PostQuitMessage(0);
		else // known but not main window
			pWindow->close();
		break;

	case WM_QUIT:
		bDefProc = false;
		break;
	}

	if (bDefProc)
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	else
		return 0;
}
