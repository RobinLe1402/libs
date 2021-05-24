/***************************************************************************************************
 FILE:	input.keyboard.hpp
 CPP:	input.keyboard.cpp
 DESCR:	Handling of keyboard input
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_INPUT_KEYBOARD
#define ROBINLE_INPUT_KEYBOARD





//==================================================================================================
// FORWARD DECLARATIONS

//--------------------------------------------------------------------------------------------------
// <stdint.h>
typedef unsigned char uint8_t;

//--------------------------------------------------------------------------------------------------
// <Windows.h>
typedef unsigned int UINT;

#ifdef _WIN64
typedef long long LPARAM;
typedef unsigned long long WPARAM;
#else
typedef long LPARAM;
typedef unsigned int WPARAM;
#endif // _WIN64



//==================================================================================================
// DECLARATION
namespace rl
{
	
	/// <summary>
	/// State of a single keyboard key
	/// </summary>
	struct KeyState
	{
		bool bPressed;
		bool bHeld; // currently held down?
		bool bReleased;
	};



	/// <summary>
	/// Handle keyboard input
	/// </summary>
	class Keyboard
	{
	public: // methods

		static Keyboard& getInstance();



		/// <summary>
		/// Update keyboard state via Windows message
		/// </summary>
		/// <returns>
		/// Was the message processed? (If true --> return 0 in <c>WindowProc</c>)
		/// </returns>
		bool update(UINT uMsg, WPARAM wParam, LPARAM lParam);

		/// <summary>
		/// Process the current input data (has to be called in order for <c>getKey()</c> to work)
		/// </summary>
		void processInput();

		/// <summary>
		/// Reset all input states<para/>
		/// Should be called when the main window regains keyboard focus after losing it
		/// </summary>
		void reset();

		/// <summary>
		/// Get the state of a single key
		/// </summary>
		/// <param name="KeyCode">= Windows keycode of desired key (<c>VK_[...]</c>)</param>
		inline KeyState getKey(uint8_t KeyCode) { return m_oKeyStates[KeyCode]; }


	private: // methods

		Keyboard(); // --> singleton
		~Keyboard();


	private: // variables

		static KeyState* m_oKeyStates;
		static bool* m_bStatesOld;
		static bool* m_bStatesNew;

	};
	
}





#endif // ROBINLE_INPUT_KEYBOARD