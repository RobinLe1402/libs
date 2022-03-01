#include "rl/input.keyboard.hpp"

#include <stdint.h>
#include <Windows.h>





namespace rl
{

	//==============================================================================================
	// STATIC VARIABLES

	KeyState* Keyboard::m_oKeyStates = nullptr;
	bool* Keyboard::m_bStatesOld = nullptr;
	bool* Keyboard::m_bStatesNew = nullptr;





	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	Keyboard::Keyboard()
	{
		m_oKeyStates = new KeyState[256];
		m_bStatesOld = new bool[256];
		m_bStatesNew = new bool[256];

		reset();
	}

	Keyboard::~Keyboard()
	{
		delete[] m_oKeyStates;
		delete[] m_bStatesOld;
		delete[] m_bStatesNew;
	}





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
			m_bStatesNew[wParam] = true;
			return true;

		case WM_KEYUP:
			m_bStatesNew[wParam] = false;
			return true;

		case WM_CHAR:
			m_cLastInput = (char32_t)wParam;
			return true;

			// Window loses focus --> reset keys to prevent messy state
		case WM_KILLFOCUS:
			reset();

		default: // non-key message
			return false;
		}
	}

	void Keyboard::processInput()
	{
		for (uint16_t i = 0; i < 256; i++)
		{
			if (m_bStatesOld[i]) // held before
			{
				m_oKeyStates[i].bPressed = false; // not newly pressed anymore

				if (!m_bStatesNew[i]) // not held now --> released
				{
					m_oKeyStates[i].bHeld = false;
					m_oKeyStates[i].bReleased = true;
				}
			}

			else // not held before
			{
				m_oKeyStates[i].bReleased = false; // not newly released anymore

				if (m_bStatesNew[i]) // held now --> newly pressed
				{
					m_oKeyStates[i].bPressed = true;
					m_oKeyStates[i].bHeld = true;
				}
			}

			m_bStatesOld[i] = m_bStatesNew[i];
		}
	}

	void Keyboard::reset()
	{
		memset(m_oKeyStates, 0, 256 * sizeof(KeyState));
		memset(m_bStatesOld, 0, 256 * sizeof(bool));
		memset(m_bStatesNew, 0, 256 * sizeof(bool));

		clearLastChar();
	}

}