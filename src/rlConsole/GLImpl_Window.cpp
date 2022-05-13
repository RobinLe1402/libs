#include "include/GLImpl_Window.hpp"

#include "include/PImpl.hpp"



ConsoleWindow::ConsoleWindow(impl::rlConsole* pConsole) : m_pConsole(pConsole)
{
	 // ToDo: ConsoleWindow constructor?
}

bool ConsoleWindow::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return m_pConsole->OnMessage(uMsg, wParam, lParam);
}
