#include "rl/input.keyboard.hpp"

#include <stdint.h>
#include <Windows.h>





namespace rl
{

	//==============================================================================================
	// STATIC VARIABLES

	KeyState Keyboard::s_oKeyStates[256] = {};
	bool Keyboard::s_bStatesOld[256] = {};
	bool Keyboard::s_bStatesNew[256] = {};





	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// STATIC METHODS

	Keyboard& Keyboard::getInstance()
	{
		static Keyboard kb;
		return kb;
	}





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	bool Keyboard::update(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_KEYDOWN:
			s_bStatesNew[wParam] = true;
			break;

		case WM_KEYUP:
			s_bStatesNew[wParam] = false;
			break;

		case WM_CHAR:
			s_cLastInput = (char32_t)wParam;
			break;

			// Window loses focus --> reset keys to prevent messy state
		case WM_KILLFOCUS:
			reset();
			break;

		default: // non-key message
			return false;
		}

		return true;
	}

	void Keyboard::processInput()
	{
		for (uint16_t i = 0; i < 256; i++)
		{
			if (s_bStatesOld[i]) // held before
			{
				s_oKeyStates[i].iState &= ~KeyStateFlags::iPressed; // not newly pressed anymore

				if (!s_bStatesNew[i]) // not held now --> released
				{
					s_oKeyStates[i].iState &= ~KeyStateFlags::iHeld;
					s_oKeyStates[i].iState |= KeyStateFlags::iReleased;
				}
			}

			else // not held before
			{
				s_oKeyStates[i].iState &= ~KeyStateFlags::iReleased; // not newly released anymore

				if (s_bStatesNew[i]) // held now --> newly pressed
				{
					s_oKeyStates[i].iState |= KeyStateFlags::iPressed;
					s_oKeyStates[i].iState |= KeyStateFlags::iHeld;
				}
			}

			s_bStatesOld[i] = s_bStatesNew[i];
		}
	}

	void Keyboard::reset()
	{
		memset(s_oKeyStates, 0, 256 * sizeof(KeyState));
		memset(s_bStatesOld, 0, 256 * sizeof(bool));
		memset(s_bStatesNew, 0, 256 * sizeof(bool));

		clearLastChar();
	}

}