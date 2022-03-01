/***************************************************************************************************
 FILE:	commandline.hpp
 CPP:	commandline.cpp
 DESCR:	Functions for parsing command line arguments
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_COMMANDLINE
#define ROBINLE_COMMANDLINE





//==================================================================================================
// FORWARD DECLARATIONS

//--------------------------------------------------------------------------------------------------
// <Windows.h>
#define MAX_PATH 260


#pragma comment(lib, "Shlwapi.lib")



//==================================================================================================
// DECLARATION
namespace rl
{
	
	/// <summary>
	/// Get the length of a filepath in the command line
	/// </summary>
	/// <param name="szCMD">= command-line, should start with the path right away</param>
	/// <returns>
	/// Was there a possible file path found?<para/>
	/// Note: The path is not validated. It could contain illegal characters/be completely invalid.
	/// </returns>
	bool CmdGetPathLen(const wchar_t* szCMD, int* pLen);

	/// <summary>
	/// Get input and output path from the command line<para/>
	/// Only works with the most basic syntax (exe inputfile outputfile)
	/// <para/>
	/// Removes quotes at beginning and end, doesn't validate paths<para/>
	/// </summary>
	/// <param name="szCMD">= the full command line, including the executable path</param>
	/// <param name="szPathIn">= the buffer that should receive the input path</param>
	/// <param name="szPathOut">= the buffer that should receive the output path</param>
	/// <returns>Could the arguments be extracted?</returns>
	bool CmdGetInOutPaths(const wchar_t* szCMD, wchar_t(&szPathIn)[MAX_PATH + 1],
		wchar_t(&szPathOut)[MAX_PATH + 1]);
	
}





#undef MAX_PATH

#endif // ROBINLE_COMMANDLINE