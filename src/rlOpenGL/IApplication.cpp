#include "rl/lib/rlOpenGL/Core.hpp"

namespace lib = rl::OpenGL;



#include <chrono>



bool lib::IApplication::s_bRunning = false;

lib::IApplication::IApplication(Window& oWindow, IRenderer& oRenderer) :
	m_oWindow(oWindow), m_oRenderer(oRenderer)
{}

bool lib::IApplication::execute(AppConfig& cfg)
{
	if (s_bRunning)
		return false;

	s_bRunning = true;


	if (!m_oWindow.create(cfg.window,
		[&](UINT uMsg, WPARAM wParam, LPARAM lParam) { winMessage(uMsg, wParam, lParam); },
		[&]() { winMinimized(); },
		[&]() { winRestored(); }))
	{
		s_bRunning = false;
		return false;
	}

	if (!m_oRenderer.create(GetDC(m_oWindow.handle()), m_oWindow.width(), m_oWindow.height(),
		cfg.renderer))
	{
		m_oWindow.destroy();
		s_bRunning = false;
		return false;
	}

	createGraph(&m_pLiveGraph);
	createGraph(&m_pGraphForRenderer);

	if (!OnStart() || !OnUpdate(0.0f)) // initialize values
	{
		m_oRenderer.destroy();
		m_oWindow.destroy();
		s_bRunning = false;
		return false;
	}
	updateRenderer(); // try to draw the first frame
	m_bMessageByApp = true;
	m_oWindow.show();
	m_bMessageByApp = false;

	auto time1 = std::chrono::system_clock::now();
	auto time2 = time1;
	std::chrono::duration<float> oElapsed;

	m_bAtomRunning = true;
	while (true)
	{
		// process Windows message (if available)
		{
			// block access to the window IO while reading from it
			std::unique_lock lm(m_muxWindow);
			handleMessage();

			if (m_bSleeping)
			{
				OnMinize();
				m_cvMinimized.wait(lm);
				m_bSleeping = false;
				OnRestore();

				time1 = std::chrono::system_clock::now();
				time2 = time1;
				oElapsed = time1 - time2;
			}
		}

		if (m_oWindow.getCloseRequested())
		{
			if (OnUserCloseQuery())
				break; // immediately exit the application loop
			else
				m_oWindow.clearCloseRequest();
		}


		// calculate time
		time2 = std::chrono::system_clock::now();
		oElapsed = time2 - time1;
		time1 = time2;

		if (OnUpdate(oElapsed.count()))
			updateRenderer();
		else
			break; // exit the application loop
	}
	m_bAtomRunning = false;
	OnStop();
	destroyGraph(m_pLiveGraph);
	destroyGraph(m_pGraphForRenderer);
	m_oRenderer.destroy();

	m_oWindow.destroy();
	s_bRunning = false;
	return true;
}

void lib::IApplication::winMinimized()
{
	std::unique_lock lm(m_muxWindow);
	m_bSleeping = true;
}

void lib::IApplication::winRestored()
{
	std::unique_lock lm(m_muxWindow);
	if (!m_bSleeping)
		return;

	m_cvMinimized.notify_one();
}

void lib::IApplication::winMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_bMessageByApp)
	{
		handleMessage();
		return;
	}

	std::unique_lock lm(m_muxWindow);

	if (m_bSleeping)
	{
		if (uMsg == WM_SIZE && wParam == SIZE_RESTORED)
			m_cvMinimized.notify_one();
		else
			return;
	}

	WindowMessage msg = { uMsg, wParam, lParam };
	m_pMessage = &msg;

	if (m_bAtomRunning)
		m_cvWinMsg.wait(lm);
}

bool lib::IApplication::handleMessage()
{
	if (!m_pMessage)
		return false; // no message

	const auto& msg = *m_pMessage;

	bool bHandled = true;
	bool bRedraw = false;
	switch (msg.uMsg)
	{
		//------------------------------------------------------------------------------------------
		// SIZE-RELATED MESSAGES

	case WM_WINDOWPOSCHANGING:
	{
		if (!m_oWindow.fullscreen())
		{
			auto& wp = *reinterpret_cast<WINDOWPOS*>(msg.lParam);

			unsigned iNewWidth = (unsigned)wp.cx;
			unsigned iNewHeight = (unsigned)wp.cy;
			m_oWindow.windowToClient(iNewWidth, iNewHeight);

			OnResizing(iNewWidth, iNewHeight);

			if (iNewWidth > INT_MAX)
				iNewWidth = INT_MAX;
			if (iNewHeight > INT_MAX)
				iNewHeight = INT_MAX;

			m_oWindow.clientToWindow(iNewWidth, iNewHeight);
			wp.cx = (int)iNewWidth;
			wp.cy = (int)iNewHeight;
		}

		break;
	}

	case WM_WINDOWPOSCHANGED:
	{
		bRedraw = true;
		auto& wp = *reinterpret_cast<const WINDOWPOS*>(msg.lParam);

		unsigned iNewWidth = (unsigned)wp.cx;
		unsigned iNewHeight = (unsigned)wp.cy;

		m_oWindow.windowToClient(iNewWidth, iNewHeight);

		m_oRenderer.resize(iNewWidth, iNewHeight);
		OnResized(iNewWidth, iNewHeight);

		break;
	}

	case WM_SIZE:
	{
		if (msg.wParam == SIZE_MINIMIZED)
		{
			m_pMessage = nullptr;
			m_cvWinMsg.notify_one();
			m_bSleeping = true;
			return true;
		}

		break;
	}

	case WM_SETFOCUS:
		OnGainFocus();
		break;

	case WM_KILLFOCUS:
		OnLoseFocus();
		break;

	default:
		bHandled = false;
		break;
	}

	if (bRedraw)
		updateRenderer(); // update display immediately

	m_pMessage = nullptr;
	m_cvWinMsg.notify_one();
	return bHandled;
}

void lib::IApplication::updateRenderer()
{
	m_oRenderer.waitForFinishedFrame();
	cacheGraph();
	m_oRenderer.update(m_pGraphForRenderer);
}
