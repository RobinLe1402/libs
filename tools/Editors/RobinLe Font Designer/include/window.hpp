#pragma once
#ifndef ROBINLE_FONT_DESIGNER__WINDOW
#define ROBINLE_FONT_DESIGNER__WINDOW





// RobinLe includes
#include "rl/lib/rlOpenGL/Core.hpp"


namespace gl = rl::OpenGL;



class CustomWindow : public gl::Window
{
private: // methods

	bool OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

};





#endif // ROBINLE_FONT_DESIGNER__WINDOW
