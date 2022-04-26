#include "include/window.hpp"

#include "rl/input.mouse.hpp"


bool CustomWindow::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (rl::Mouse::getInstance().update(uMsg, wParam, lParam))
		return true;

	return false;
}
