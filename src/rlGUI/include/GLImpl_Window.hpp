#pragma once
#ifndef ROBINLE_LIB_GUI_GLIMPL_WINDOW
#define ROBINLE_LIB_GUI_GLIMPL_WINDOW





#include "rl/lib/rlOpenGL/Core.hpp"

#include <cassert>
#include <functional>

namespace GL = rl::OpenGL;



class GLImpl_Window : public GL::Window
{
public: // types

	using OnMessageEvent = std::function<bool(UINT uMsg, WPARAM wParam, LPARAM lParam)>;


public: // methods

	GLImpl_Window(OnMessageEvent fnOnMessage);
	virtual ~GLImpl_Window() = default;


private: // methods

	bool OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;


private: // variables

	OnMessageEvent m_fnOnMessage;

};





#endif // ROBINLE_LIB_GUI_GLIMPL_WINDOW
