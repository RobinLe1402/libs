#include "rl/lib/OpenGLWin.hpp"

namespace lib = rl::OpenGLWin;



#include <chrono>



bool lib::IApplication::s_bRunning = false;

bool lib::IApplication::execute(OpenGLWinConfig& cfg, Window& oWindow, IRenderer& oRenderer)
{
	if (s_bRunning)
		return false;

	s_bRunning = true;


	m_pWindow = &oWindow;
	m_pRenderer = &oRenderer;

	if (!oWindow.create(cfg.window))
	{
		m_pRenderer = nullptr;
		m_pWindow = nullptr;
		s_bRunning = false;
		return false;
	}

	if (!oRenderer.create(GetDC(oWindow.handle()), cfg.renderer.bVSync))
	{
		oWindow.destroy();
		m_pRenderer = nullptr;
		m_pWindow = nullptr;
		s_bRunning = false;
		return false;
	}

	if (!OnStart() || !OnUpdate(0.0f)) // try to draw first frame
	{
		oRenderer.destroy();
		oWindow.destroy();
		m_pRenderer = nullptr;
		m_pWindow = nullptr;
		s_bRunning = false;
		return false;
	}
	oWindow.show();

	auto time1 = std::chrono::system_clock::now();
	auto time2 = time1;
	std::chrono::duration<float> oElapsed;

	m_bAtomRunning = true;
	while (m_bAtomRunning)
	{
		// calculate time
		time2 = std::chrono::system_clock::now();
		oElapsed = time2 - time1;
		time1 = time2;

		{
			// block access to m_bAtomRunning while accessing it
			std::unique_lock lm(m_muxWindow);

			handleMessage();

			if (!m_bAtomRunning || !OnUpdate(oElapsed.count()))
			{
				m_bAtomRunning = !OnStop();

				if (m_bAtomRunning) // quit request denied --> notify window instantly
					m_cvWinClose.notify_one();
			}
		}
		

		if (m_bAtomRunning)
			m_pRenderer->update();
	}
	oRenderer.destroy();

	// in case the window sent the quit request, notify it after destroying the renderer
	m_cvWinClose.notify_one();

	oWindow.destroy();
	m_pRenderer = nullptr;
	m_pWindow = nullptr;
	s_bRunning = false;
	return true;
}

bool lib::IApplication::winClose()
{
	std::unique_lock lm(m_muxWindow);
	m_bAtomRunning = false;
	m_cvWinClose.wait(lm);
	return !m_bAtomRunning;
}

void lib::IApplication::winMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	std::unique_lock lm(m_muxWindow);

	WindowMessage msg = { uMsg, wParam, lParam };
	m_pMessage = &msg;

	if (m_bAtomRunning)
		m_cvWinMsg.wait(lm);
}

void lib::IApplication::handleMessage()
{
	if (!m_pMessage)
		return; // no message

	const auto& msg = *m_pMessage;

	switch (msg.uMsg)
	{
	case WM_SIZE:
		m_pRenderer->resize(HIWORD(msg.lParam), LOWORD(msg.lParam));
		break;

	case WM_SIZING:
	{
		const unsigned iOldNativeWidth = m_pWindow->nativeWidth();
		const unsigned iOldNativeHeight = m_pWindow->nativeHeight();

		auto& rectNew = *reinterpret_cast<RECT*>(msg.lParam);
		unsigned iNewNativeWidth = rectNew.right - rectNew.left;
		unsigned iNewNativeHeight = rectNew.bottom - rectNew.top;

		unsigned iNewClientWidth = iNewNativeWidth;
		unsigned iNewClientHeight = iNewNativeHeight;
		m_pWindow->windowToClient(iNewClientWidth, iNewClientHeight);

		LONG iCustomWidth = (LONG)iNewClientWidth;
		LONG iCustomHeight = (LONG)iNewClientHeight;
		OnResize(iCustomWidth, iCustomHeight);
		if (iCustomWidth >= 0 && iCustomHeight >= 0)
		{
			const int64_t iDiffX = (uint64_t)iCustomWidth - iNewClientWidth;
			const int64_t iDiffY = (uint64_t)iCustomHeight - iNewClientHeight;

			if (iDiffX || iDiffY)
			{
				const bool bRight =
					(msg.wParam == WMSZ_TOPRIGHT || msg.wParam == WMSZ_RIGHT ||
						msg.wParam == WMSZ_BOTTOMRIGHT);
				const bool bBottom =
					(msg.wParam == WMSZ_BOTTOM || msg.wParam == WMSZ_BOTTOMLEFT ||
						msg.wParam == WMSZ_BOTTOMRIGHT);

				if (bRight)
					rectNew.right = LONG((int64_t)rectNew.right + iDiffX);
				else
					rectNew.left = LONG((int64_t)rectNew.left - iDiffX);

				if (bBottom)
					rectNew.bottom = LONG((int64_t)rectNew.bottom + iDiffY);
				else
					rectNew.top = LONG((int64_t)rectNew.top - iDiffY);
			}
		}

		break;
	}
	}

	m_pRenderer->update(); // update display immediately

	m_pMessage = nullptr;
	m_cvWinMsg.notify_one();
}
