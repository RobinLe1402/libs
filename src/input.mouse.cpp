#include "rl/input.mouse.hpp"

#include <stdint.h>
#include <Windows.h>





#define RL_MOUSE_L 0
#define RL_MOUSE_R 1
#define RL_MOUSE_M 2

namespace rl
{

	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	Mouse::Mouse()
	{
		m_bStatesOld = new bool[3];
		m_bStatesNew = new bool[3];
		m_bDoubleClicked = new bool[3];

		memset(m_bStatesOld, 0, sizeof(bool) * 3);
		memset(m_bStatesNew, 0, sizeof(bool) * 3);
		memset(m_bDoubleClicked, 0, sizeof(bool) * 3);

		m_oState = new MouseState{};
	}

	Mouse::~Mouse()
	{
		delete[] m_bStatesOld;
		delete[] m_bStatesNew;
		delete[] m_bDoubleClicked;

		delete m_oState;
	}





	//----------------------------------------------------------------------------------------------
	// STATIC METHODS

	Mouse& Mouse::getInstance()
	{
		static Mouse m_oMouse;
		return m_oMouse;
	}





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	void Mouse::setHWND(HWND hWnd)
	{
		m_hWnd = hWnd;
		m_bMouseTracking = false;
	}

	bool Mouse::update(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (!m_bOnClient && uMsg != WM_MOUSEMOVE)
			return false;


		switch (uMsg)
		{
		case WM_MOUSEMOVE:
			m_bOnClient = true;
			m_iX = LOWORD(lParam);
			m_iY = HIWORD(lParam);

			if (!m_bMouseTracking) // enable mouse tracking on first call
			{
				TRACKMOUSEEVENT tme{};
				tme.cbSize = sizeof(tme);
				tme.hwndTrack = m_hWnd;
				tme.dwFlags = TME_LEAVE;
				
				TrackMouseEvent(&tme);

				m_bMouseTracking = true;
			}
			return true;


		case WM_MOUSELEAVE:
			m_bOnClient = false;
			m_bMouseTracking = false; // must re-call TrackMouseEvent
			return true;





			// left
		case WM_LBUTTONDOWN:
			m_bStatesNew[RL_MOUSE_L] = true;
			beginCapture();
			return true;
		case WM_LBUTTONUP:
			m_bStatesNew[RL_MOUSE_L] = false;
			endCapture();
			return true;
		case WM_LBUTTONDBLCLK:
			m_bDoubleClicked[RL_MOUSE_L] = true;
			return true;


			// right
		case WM_RBUTTONDOWN:
			m_bStatesNew[RL_MOUSE_R] = true;
			beginCapture();
			return true;
		case WM_RBUTTONUP:
			m_bStatesNew[RL_MOUSE_R] = false;
			endCapture();
			return true;
		case WM_RBUTTONDBLCLK:
			m_bDoubleClicked[RL_MOUSE_R] = true;
			return true;


			// middle
		case WM_MBUTTONDOWN:
			m_bStatesNew[RL_MOUSE_M] = true;
			beginCapture();
			return true;
		case WM_MBUTTONUP:
			m_bStatesNew[RL_MOUSE_M] = false;
			endCapture();
			return true;
		case WM_MBUTTONDBLCLK:
			m_bDoubleClicked[RL_MOUSE_M] = true;
			return true;


			// wheel
		case WM_MOUSEWHEEL:
			m_iWheelRotation += int16_t(HIWORD(wParam)) / WHEEL_DELTA;
			return true;


		case WM_KILLFOCUS:
			reset();
			return true;





		default:
			return false;
		}
	}

	void Mouse::processInput()
	{
		// mouse position
		m_oState->bOnClient = m_bOnClient;
		m_oState->x = m_iX;
		m_oState->y = m_iY;

		if (m_iX < 0 || m_iY < 0)
			m_oState->bOnClient = false;

		if (m_oState->bOnClient && m_hWnd)
		{
			RECT rectClient{};  
			if (GetClientRect(m_hWnd, &rectClient))
			{
				const auto iClientWidth  = rectClient.right  - rectClient.left;
				const auto iClientHeight = rectClient.bottom - rectClient.top;

				if (m_iX > iClientWidth || m_iY > iClientHeight)
					m_oState->bOnClient = false;
			}
		}

		// mouse wheel
		m_oState->iWheelRotation = m_iWheelRotation;
		m_iWheelRotation = 0;

		// key states
		MouseKeyState* pStates[3] = { &m_oState->left, &m_oState->right, &m_oState->middle };
		for (uint8_t i = 0; i < 3; i++)
		{

			if (m_bStatesOld[i]) // held before
			{
				pStates[i]->bPressed = false; // not newly pressed anymore

				if (!m_bStatesNew[i]) // not held now --> released
				{
					pStates[i]->bHeld = false;
					pStates[i]->bReleased = true;
				}
			}

			else // not held before
			{
				pStates[i]->bReleased = false; // not newly released anymore

				if (m_bStatesNew[i]) // held now --> newly pressed
				{
					pStates[i]->bPressed = true;
					pStates[i]->bHeld = true;
				}
			}

			m_bStatesOld[i] = m_bStatesNew[i];

			pStates[i]->bDoubleClicked = m_bDoubleClicked[i];
			m_bDoubleClicked[i] = false;

		}
	}

	void Mouse::reset()
	{
		for (uint8_t i = 0; i < 3; i++)
		{
			m_bStatesNew[i] = false;
			m_bDoubleClicked[i] = false;
		}
		m_iWheelRotation = 0;
	}





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	void Mouse::beginCapture()
	{
		if (!m_hWnd)
			return;

		++m_iCaptureCount;
		if (m_iCaptureCount == 1)
			SetCapture(m_hWnd);
	}

	void Mouse::endCapture()
	{
		if (m_iCaptureCount == 0)
			return;

		--m_iCaptureCount;
		if (m_iCaptureCount == 0)
			ReleaseCapture();
	}

}