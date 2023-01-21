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
using uint8_t = unsigned char;

//--------------------------------------------------------------------------------------------------
// <Windows.h>
using UINT = unsigned int;

#ifdef _WIN64
using LPARAM = long long;
using WPARAM = unsigned long long;
#else
using LPARAM = long;
using WPARAM = unsigned int;
#endif // _WIN64



//==================================================================================================
// DECLARATION
namespace rl
{

	namespace KeyStateFlags
	{
		constexpr uint8_t iPressed = 0x01;
		constexpr uint8_t iHeld = 0x02;
		constexpr uint8_t iReleased = 0x04;
	}

	/// <summary>
	/// State of a single keyboard key
	/// </summary>
	struct KeyState
	{
		uint8_t iState;

		bool pressed() const { return iState & KeyStateFlags::iPressed; }
		bool held() const { return iState & KeyStateFlags::iHeld; }
		bool released() const { return iState & KeyStateFlags::iReleased; }
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
		inline KeyState getKey(uint8_t KeyCode) const { return s_oKeyStates[KeyCode]; }

		/// <summary>
		/// Get the last input character (0 = no value)
		/// </summary>
		inline char32_t getLastChar() const { return s_cLastInput; }

		/// <summary>
		/// Clear the last input character (set it to 0)
		/// </summary>
		inline void clearLastChar() { s_cLastInput = 0; }


	private: // methods

		Keyboard() = default; // --> singleton
		~Keyboard() = default;


	private: // variables

		static KeyState s_oKeyStates[256];
		static bool s_bStatesOld[256];
		static bool s_bStatesNew[256];
		char32_t s_cLastInput = 0;

	};

}





#endif // ROBINLE_INPUT_KEYBOARD