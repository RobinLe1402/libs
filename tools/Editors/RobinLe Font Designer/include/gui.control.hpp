#pragma once
#ifndef ROBINLE_FONT_DESIGNER__GUI_CONTROL
#define ROBINLE_FONT_DESIGNER__GUI_CONTROL





#include "rl/input.keyboard.hpp"
#include "rl/input.mouse.hpp"



namespace GUI
{

	class IControl
	{
	public: // methods

		IControl(unsigned iX, unsigned iY, unsigned iWidth, unsigned iHeight, bool bVisible,
			bool bFocus = false) :
			m_bFocus(bFocus), m_bVisible(bVisible),
			m_iX(iX), m_iY(iY),
			m_iWidth(iWidth), m_iHeight(iHeight)
		{ }
		virtual ~IControl() = default;

		// Use rl::Keyboard and rl::Mouse to determine the new state of the control.
		// Called when the control is focused.
		virtual void handleInput() = 0;

		auto hasFocus() const { return m_bFocus; }
		auto isVisible() const { return m_bVisible; }
		auto getX() const { return m_iX; }
		auto getY() const { return m_iY; }
		auto getWidth() const { return m_iWidth; }
		auto getHeight() const { return m_iHeight; }
		auto getVisible() const { return m_bVisible; }

		void setFocused(bool bFocused) { m_bFocus = bFocused; }
		void setVisible(bool bVisible) { m_bVisible = bVisible; }
		void setX(unsigned iX) { m_iX = iX; }
		void setY(unsigned iY) { m_iX = iY; }


	protected: // variables

		bool m_bFocus;
		bool m_bVisible;

		unsigned m_iX, m_iY;
		unsigned m_iWidth, m_iHeight;
	};

}





#endif // ROBINLE_FONT_DESIGNER__GUI_CONTROL