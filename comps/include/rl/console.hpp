/***************************************************************************************************
 FILE:	console.hpp
 CPP:	console.cpp
 DESCR:	Helper singleton for working with the Windows console
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_CONSOLE
#define ROBINLE_CONSOLE





//==================================================================================================
// FORWARD DECLARATIONS

//--------------------------------------------------------------------------------------------------
// <stdint.h>
typedef unsigned char uint8_t;

//--------------------------------------------------------------------------------------------------
// <Windows.h>
typedef void* HANDLE;
typedef unsigned int UINT;


#include <vector>



//==================================================================================================
// DECLARATION
namespace rl
{

#define FG_BLACK		0x00
#define FG_DARKBLUE		0x01
#define FG_DARKGREEN	0x02
#define FG_DARKCYAN		0x03
#define FG_DARKRED		0x04
#define FG_DARKMAGENTA	0x05
#define FG_DARKYELLOW	0x06
#define FG_GRAY			0x07
#define FG_DARKGRAY		0x08
#define FG_BLUE			0x09
#define FG_GREEN		0x0A
#define FG_CYAN			0x0B
#define FG_RED			0x0C
#define FG_MAGENTA		0x0D
#define FG_YELLOW		0x0E
#define FG_WHITE		0x0F

#define BG_BLACK		0x00
#define BG_DARKBLUE		0x10
#define BG_DARKGREEN	0x20
#define BG_DARKCYAN		0x30
#define BG_DARKRED		0x40
#define BG_DARKMAGENTA	0x50
#define BG_DARKYELLOW	0x60
#define BG_GRAY			0x70
#define BG_DARKGRAY		0x80
#define BG_BLUE			0x90
#define BG_GREEN		0xA0
#define BG_CYAN			0xB0
#define BG_RED			0xC0
#define BG_MAGENTA		0xD0
#define BG_YELLOW		0xE0
#define BG_WHITE		0XF0



	/// <summary>
	/// A helper class for dealing with the Windows console<para/>
	/// Automatically resets changes when application execution ends
	/// </summary>
	class Console final
	{
	public: // methods

		/// <summary>
		/// Set the text color
		/// </summary>
		/// <param name="color">
		/// = A <c>FG_</c> define/a <c>BG_</c> define/a combination of both
		/// </param>
		static void PushColor(uint8_t color);

		/// <summary>
		/// Resets the text color to before the last <c>PushColor()</c>
		/// </summary>
		static void PopColor();

		/// <summary>
		/// Resets the text color to the startup value
		/// </summary>
		static void ResetColor();


		/// <summary>
		/// Switch the console's codepage
		/// </summary>
		static void SetCodepage(UINT cp);

		/// <summary>
		///  Reset the console's codepage to the startup value
		/// </summary>
		static void ResetCodepage();


	private: // methods

		Console(); // --> singleton
		~Console();

		static void setColor(uint8_t color);

		
	private: // variables

		static std::vector<uint8_t> m_oColorStack;
		static uint8_t m_iStartupColor;
		static UINT m_iStartupCP;
		static bool m_bPushed;

		static Console m_oInstance; // singleton
		static HANDLE m_hOut;

	};

}





#endif // ROBINLE_CONSOLE