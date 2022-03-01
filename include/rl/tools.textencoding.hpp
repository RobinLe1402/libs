/***************************************************************************************************
 FILE:	tools.textencoding.hpp
 CPP:	<n/a>
 DESCR:	Help functions for working with text encodings
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_TOOLS_TEXTENCODING
#define ROBINLE_TOOLS_TEXTENCODING






//==================================================================================================
// FORWARD DECLARATIONS

//--------------------------------------------------------------------------------------------------
// <stdint.h>
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;


#include <Windows.h>



//==================================================================================================
// DECLARATION
namespace rl
{
	const uint16_t HighSurrogate = 0b11011000;
	const uint16_t LowSurrogate = 0b11011100;



	uint32_t DecodeWideChar(const wchar_t* ch)
	{
		uint16_t iVal = ch[0];
		if (((iVal >> 8) & 0xFC) != HighSurrogate) // single WORD
		{
			return (uint32_t)iVal;
		}
		else // double WORD
		{
			uint16_t iVal2 = ch[1];
			if (((iVal2 >> 8) & 0xFC) != LowSurrogate) // No low surrogate --> invalid encoding
				throw std::exception("Invalid UTF-16 encoding");

			unsigned int iResult = (iVal & 0x03FF) << 10;
			iResult |= (iVal2 & 0x03FF);
			iResult += 0x010000;

			return iResult;
		}
	}

	uint32_t DecodeChar(char ch)
	{
		char sz[2] = { ch, 0 };
		wchar_t szw[3];
		MultiByteToWideChar(CP_ACP, 0, sz, 1, szw, 2);
		return DecodeWideChar(szw);
	}

}





// #undef foward declared definitions

#endif // ROBINLE_TOOLS_TEXTENCODING