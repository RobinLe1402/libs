/***************************************************************************************************
 FILE:	unicode.hpp
 CPP:	unicode.cpp
 DESCR:	Some functions providing information about Unicode code points
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_UNICODE
#define ROBINLE_UNICODE



#if __cplusplus > 201703L
#include <memory>
#endif // C++17





//==================================================================================================
// DECLARATION
namespace rl
{
	namespace Unicode
	{

		/// <summary>
		/// Is a Unicode character located inside of a private use area?
		/// </summary>
		bool IsPrivateUse(char32_t ch) noexcept;

		/// <summary>
		/// Is a value a noncharacter in Unicode?
		/// </summary>
		bool IsNoncharacter(char32_t ch) noexcept;

	}

	namespace UTF16
	{

		/// <summary>
		/// Is a UTF-16 codeunit a high surrogate (the first part of a surrogate pair)?
		/// </summary>
		inline constexpr bool IsHighSurrogate(char16_t ch) noexcept
		{
			return ((ch & 0xFC00) == 0xD800);
		}

		/// <summary>
		/// Is a UTF-16 codeunit a low surrogate (the second part of a surrogate pair)?
		/// </summary>
		inline constexpr bool IsLowSurrogate(char16_t ch) noexcept
		{
			return ((ch & 0xFC00) == 0xDC00);
		}

		/// <summary>
		/// Decode a UTF-16 character<para />
		/// <b>Warning:</b> Doesn't check if the codeunits are actually surrogates
		/// </summary>
		char32_t Decode(char16_t hi, char16_t lo) noexcept;



#if __cplusplus > 201703L

		/// <summary>
		/// Decode a UTF-16 string to UTF-32<para />
		/// Throws an <c>std::exception</c> if the input string is not valid
		/// </summary>
		/// <param name="len">
		/// | If not <c>nullptr</c>, the pointed to variable receives the length, in characters, of
		/// the decoded string (without the terminating zero)
		/// </param>
		std::shared_ptr<char32_t[]> DecodeString(const char16_t* sz, size_t* len = nullptr);

#ifdef _WIN32
		/// <summary>
		/// Decode a wide string to UTF-32<para />
		/// Throws an <c>std::exception</c> if the input string is not valid
		/// </summary>
		/// <param name="len">= The length, in characters, of the decoded string</param>
		inline auto DecodeString(const wchar_t* sz, size_t* len = nullptr)
		{
			return DecodeString((const char16_t*)sz, len);
		}
#endif // _WIN32

#endif // C++17

	}
}





#endif // ROBINLE_UNICODE