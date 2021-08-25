/***************************************************************************************************
 FILE:	UnicodeData.hpp
 DLL:	UnicodeData.dll
 DESCR:	Get information about unicode characters (based on "UnicodeData.txt")
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_DLL_UNICODEDATA
#define ROBINLE_DLL_UNICODEDATA

#ifdef LIBRARY_EXPORTS
	#define UNICODEDATA_API extern "C" __declspec(dllexport)
#else
	#define UNICODEDATA_API extern "C" __declspec(dllimport)
	#pragma comment(lib, "UnicodeData.lib")
#endif





//==================================================================================================
// DECLARATION
namespace rl
{
	namespace UnicodeDataDLL
	{

		using UChar_t = unsigned int; // 32 bit
		using NameLen_t = unsigned short; // 16 bit





		/// <summary>
		/// Get the required length, in bytes, for the name of a character
		/// </summary>
		/// <param name="ch">= raw unicode value of the character</param>
		/// <returns>
		/// <c>0</c>: The character is not defined<para/>
		/// <c>1</c>: The character has no name defined<para/>
		/// <c>&gt;1</c>: The required buffer size, in bytes (including terminating zero)
		/// </returns>
		UNICODEDATA_API NameLen_t __stdcall GetNameLen(UChar_t ch);



		
		/// <summary>
		/// Get the name of a unicode character
		/// </summary>
		/// <param name="ch">= raw unicode value of the character</param>
		/// <param name="buf">= the buffer that should receive the name (pure ASCII)</param>
		/// <param name="buf_size">
		/// = the size of the buffer pointed to by <c>buf</c> (includes terminating zero)
		/// </param>
		/// <returns>
		/// If the function fails, zero<para/>
		/// If the function succeeded, the count of characters written to <c>buf</c>, in bytes
		/// (including terminating zero)
		/// </returns>
		UNICODEDATA_API NameLen_t __stdcall GetName(UChar_t ch, char* buf, NameLen_t buf_size);

	}
}





#endif // ROBINLE_DLL_UNICODEDATA