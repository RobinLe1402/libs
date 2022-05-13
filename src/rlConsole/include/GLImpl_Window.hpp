#pragma once
#ifndef ROBINLE_CONSOLE_GLIMPL_WINDOW
#define ROBINLE_CONSOLE_GLIMPL_WINDOW





#include "rl/lib/rlOpenGL/Core.hpp"

namespace GL = rl::OpenGL;

namespace impl
{
	class rlConsole; // forward declaration for "PImpl.h"
}

class ConsoleWindow : public GL::Window
{
public: // methods

	ConsoleWindow(impl::rlConsole* pConsole);
	virtual ~ConsoleWindow() = default;


private: // methods

	bool OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;


private: // variables

	impl::rlConsole* m_pConsole;
};





#endif // ROBINLE_CONSOLE_GLIMPL_WINDOW
