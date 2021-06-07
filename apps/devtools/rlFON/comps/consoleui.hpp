/***************************************************************************************************
 FILE:	consoleui.hpp
 CPP:	consoleui.cpp
 DESCR:	Some global functions for use in the rlFON developer applications
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_FON_CONSOLEUI
#define ROBINLE_FON_CONSOLEUI





//==================================================================================================
// FORWARD DECLARATIONS

//--------------------------------------------------------------------------------------------------
// <stdint.h>
typedef unsigned char uint8_t;



//==================================================================================================
// DECLARATION
namespace rl
{

	/// <summary>
	/// Write a warning to the console
	/// </summary>
	void WriteWarning(const char* szText, ...);

	/// <summary>
	/// Write an error to the console
	/// </summary>
	void WriteError(const char* szText, ...);

	/// <summary>
	/// Write a hint towards the "/?" or "--help" parameter after a syntax error has occured
	/// </summary>
	/// <param name="szExeFilename">= The EXE filename as it should show up in the example</param>
	void WriteHelpHint(const char* szExeFilename);



	/// <summary>
	/// Write all set warning flags of the FON parser to the console
	/// </summary>
	void WriteFONWarnings(uint8_t iWarningFlags);

	/// <summary>
	/// Write a FON parser error to the console
	/// </summary>
	void WriteFONError(uint8_t iError);
	
}





#endif // ROBINLE_FON_CONSOLEUI