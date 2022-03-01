/***************************************************************************************************
 FILE:	input.mouse.hpp
 CPP:	input.mouse.cpp
 DESCR:	Handling of mouse input
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_INPUT_MOUSE
#define ROBINLE_INPUT_MOUSE





//==================================================================================================
// FORWARD DECLARATIONS

//--------------------------------------------------------------------------------------------------
// <stdlib.h>
typedef unsigned short uint16_t;
typedef short int16_t;


#include <Windows.h>



//==================================================================================================
// DECLARATION
namespace rl
{
	
	/// <summary>
	/// State of a single mouse key
	/// </summary>
	struct MouseKeyState
	{
		bool bPressed;
		bool bHeld;
		bool bReleased;
		bool bDoubleClicked; // requires CS_DBLCLKS in WNDCLASS.style
	};



	/// <summary>
	/// State of the whole mouse
	/// </summary>
	struct MouseState
	{
		MouseKeyState left;
		MouseKeyState right;
		MouseKeyState middle;

		/// <summary>
		/// Value between -273 and +273.<para/>
		/// &gt; 0 --> forward/up<para/>
		/// &lt; 0 --> backward/down
		/// </summary>
		int16_t iWheelRotation;

		bool bOnClient;
		uint16_t x, y; // valid if bOnClient is true
	};





	/// <summary>
	/// Handle mouse input
	/// </summary>
	class Mouse
	{
	public:

		static Mouse& getInstance();



		/// <summary>
		/// Change the tracked window (for <c>WM_MOUSELEAVE</c>)
		/// </summary>
		void setHWND(HWND hWnd);
		
		/// <summary>
		/// Update mouse state via Windows message
		/// </summary>
		/// <returns>
		/// Was the message processed? (If true --> return 0 in <c>WindowProc</c>)
		/// </returns>
		bool update(UINT uMsg, WPARAM wParam, LPARAM lParam);
		
		/// <summary>
		/// Process the current input data (has to be called in order for <c>getState()</c> to work)
		/// </summary>
		void processInput();

		/// <summary>
		/// Get the current state of the mouse
		/// </summary>
		inline const MouseState& getState() { return *m_oState; }

		/// <summary>
		/// Reset all input states<para/>
		/// Should be called when the main window loses mouse focus
		/// </summary>
		void reset();


	private: // methods

		Mouse(); // --> singleton
		~Mouse();


	private: // variables

		bool* m_bStatesOld = nullptr;
		bool* m_bStatesNew = nullptr;
		bool* m_bDoubleClicked = nullptr;
		int16_t m_iWheelRotation = 0;

		bool m_bOnClient = false;
		uint16_t m_iX = 0, m_iY = 0;
		bool m_bMouseTracking = false;

		HWND m_hWnd = NULL; // for mouse exit/enter tracking
		MouseState* m_oState = nullptr; // heap for thread safety

	};
	
}





#endif // ROBINLE_INPUT_MOUSE