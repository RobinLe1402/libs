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


//--------------------------------------------------------------------------------------------------
// <Windows.h>
#ifndef _MINWINDEF_
#define MAX_PATH 260
#endif // _MINWINDEF_



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



	/// <summary>
	/// Get a file/directory path from the command line<para/>
	/// The path must be in the standard Windows format, i.e. either with no spaces or wrapped
	/// with quotation marks.<para/>
	/// The path's content is not validated.</para/>
	/// The path may start after a spaceless prefix (hence <c>startindex</c>), but it can't have a
	/// spaceless suffix.
	/// </summary>
	/// <param name="startarg">
	/// = index (starts after EXE file path, 0 means argv[1] etc) of the parameter where the path
	/// starts
	/// </param>
	/// <param name="startindex">
	/// = index of the first character of the path in<c>startarg</c>
	/// </param>
	/// <param name="dest">
	/// = destination for the full path to be copied into
	/// </param>
	/// <returns>
	/// If the function succeeds, it returns the count of arguments that the path used up.<para/>
	/// If the function fails (because either the parameters are invalid or the path isn't formatted
	/// properly), it returns 0.
	/// </returns>
	int CmdGetPath(int argc, wchar_t* argv[], int startarg, size_t startindex,
		wchar_t(&dest)[MAX_PATH + 1]);

}





#ifndef _MINWINDEF_
#undef MAX_PATH
#endif // _MINWINDEF_

#endif // ROBINLE_FON_CONSOLEUI