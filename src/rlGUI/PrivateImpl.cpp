#include "include/PrivateImpl.hpp"



lib::PrivateImpl::PrivateImpl(
	IGUIApplication& oApplication, GUIRenderer& oRenderer, GUIWindow& oWindow)
	:
	m_oApplication(oApplication), m_oRenderer(oRenderer), m_oWindow(oWindow),
	m_oGLWindow([&](UINT uMsg, WPARAM wParam, LPARAM lParam) -> bool
		{ return m_oWindow.OnMessage(uMsg, wParam, lParam); }),
	m_oGLRenderer([&](const lib::GUIGraph& oGraph)
		{ m_oRenderer.render(oGraph); }),
	m_oGLApplication(this, m_oGLWindow, m_oGLRenderer)
{
	// ToDo:
	m_oRenderer.m_pPImpl = this;
	m_oWindow.m_pPImpl = this;
}
