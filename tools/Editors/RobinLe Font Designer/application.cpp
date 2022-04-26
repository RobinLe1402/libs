#include "include/application.hpp"

#include "include/constants.hpp"
#include "include/font.hpp"

#include "rl/input.mouse.hpp"

#include <cstdint>





bool Application::OnStart()
{
	auto& oGraph = *static_cast<Graph*>(graph());
	oGraph.iPixelScale = 1;
	oGraph.sStatus = "Nothing loaded";

	LONG iWidth = window().width();
	LONG iHeight = window().height();
	OnResize(iWidth, iHeight);

	rl::SplashScreen::Close();
	return true;
}

bool Application::OnUpdate(float fElapsedTime)
{
	auto& oGraph = *static_cast<Graph*>(graph());
	auto& oMouse = rl::Mouse::getInstance();

	oMouse.processInput();
	auto& oMouseState = oMouse.getState();

	const auto iWidth = window().width();
	const auto iHeight = window().height();


	const auto eStatePrev = oGraph.btPrev.state;
	const auto eStateNext = oGraph.btNext.state;
	oGraph.btPrev.state = ButtonState::Normal;
	oGraph.btNext.state = ButtonState::Normal;

	if (oMouseState.bOnClient)
	{
		const unsigned iX = oMouseState.x / oGraph.iPixelScale;
		const unsigned iY = oMouseState.y / oGraph.iPixelScale;

		const unsigned iFooterClientY = iHeight - iFooterHeight;
		
		// mouse in footer button area?
		if (iY >= iFooterClientY + iFooterButtonTop && iY <= iFooterClientY + iFooterButtonBottom &&
			iX >= iWidth - iFooterButton_Prev_L && iX <= iWidth - iFooterButton_Next_R)
		{
			int8_t iButton = -1;

			if (iX <= iFooterButton_Prev_R)
				iButton = 0;
			else if (iX >= iFooterButton_Next_L && iX <= iFooterButton_Next_R)
				iButton = 1;

			Button* pButton = nullptr;
			ButtonState eState{};

			if (iButton == 0)
			{
				pButton = &oGraph.btPrev;
				eState = eStatePrev;
			}
			else
			{
				pButton = &oGraph.btNext;
				eState = eStateNext;
			}

			if (pButton)
			{
				pButton->state = ButtonState::Hovering;
				if (oMouseState.left.bHeld &&
					(eState == ButtonState::Hovering || eState == ButtonState::Clicked))
					pButton->state = ButtonState::Clicked;

				else if (oMouseState.left.bReleased)
				{
					pButton->state = ButtonState::Hovering;
					// ToDo: OnClick
				}
			}
		}
	}

	// ToDo: Handle the mouse state

	return true;
}

void Application::OnResize(LONG& iWidth, LONG& iHeight)
{
	auto& oGraph = *static_cast<Graph*>(graph());

	iWidth -= iWidth % oGraph.iPixelScale;
	iHeight -= iHeight % oGraph.iPixelScale;

	oGraph.btPrev.rect.top = oGraph.btNext.rect.top = iHeight - 3 - 26;
	oGraph.btPrev.rect.bottom = oGraph.btNext.rect.bottom = iHeight - 3;
}
