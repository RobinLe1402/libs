#include "include/GLImpl_Window.hpp"



GLImpl_Window::GLImpl_Window(OnMessageEvent fnOnMessage) : m_fnOnMessage(fnOnMessage)
{
	assert(fnOnMessage != nullptr);
}

bool GLImpl_Window::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return m_fnOnMessage(uMsg, wParam, lParam);
}
