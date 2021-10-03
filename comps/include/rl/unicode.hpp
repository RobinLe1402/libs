/***************************************************************************************************
 FILE:	unicode.hpp
 CPP:	unicode.cpp
 DESCR:	Some functions providing information about Unicode code points
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_UNICODE
#define ROBINLE_UNICODE





//==================================================================================================
// DECLARATION
namespace rl
{
	namespace Unicode
	{

		/// <summary>
		/// Is a Unicode character located inside of a private use area?
		/// </summary>
		bool IsPrivateUse(char32_t ch);

		/// <summary>
		/// Is a value a noncharacter in Unicode?
		/// </summary>
		bool IsNoncharacter(char32_t ch);

	}
}





#endif // ROBINLE_UNICODE