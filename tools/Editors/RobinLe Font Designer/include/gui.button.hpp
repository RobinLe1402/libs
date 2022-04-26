#pragma once
#ifndef ROBINLE_FONT_DESIGNER__GUI_BUTTON
#define ROBINLE_FONT_DESIGNER__GUI_BUTTON





#include "gui.control.hpp"
#include "constants.hpp"



namespace GUI
{

	enum class ButtonState
	{
		Normal,
		Disabled,
		Hover
	};
	
	class Button : public IControl
	{
	public: // methods

		Button(unsigned iX, unsigned iY, unsigned iWidth, bool bVisible, bool bEnabled) :
			IControl(iX, iY, iWidth, iButtonHeight, bVisible),
			m_eState(bEnabled ? ButtonState::Normal : ButtonState::Disabled)
		{}

		inline auto getState() const { return m_eState; }

		// ToDo: void handleInput() override .........


	private: // variables

		ButtonState m_eState;

	};

}





#endif // ROBINLE_FONT_DESIGNER__GUI_BUTTON
