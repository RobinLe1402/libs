#include "rl/unicode.hpp"







namespace rl
{

	namespace Unicode
	{

		bool IsPrivateUse(char32_t ch) noexcept
		{
			return ((ch >= 0xE000 && ch <= 0xF8FF) /* Private Use Zone */ ||
				(ch >= 0xF0000 && ch < 0x110000) /* Private Use Planes */);
		}

		bool IsNoncharacter(char32_t ch) noexcept
		{
			return ((ch >= 0xFDD0 && ch <= 0xFDEF) /* noncharacter range in BMP */ ||
				(ch & 0xFFFF) >= 0xFFFE /* last two code points of each plane */);
		}

	}

	namespace UTF16
	{

		char32_t Decode(char16_t hi, char16_t lo) noexcept
		{
			char32_t result = lo & 0x03FF;
			result |= (char32_t)(hi & 0x03FF) << 10;
			result += 0x10000;

			return result;
		}
		
		
#if __cplusplus > 201703L

		std::shared_ptr<char32_t[]> DecodeString(const char16_t* sz, size_t* len)
		{
			size_t idx = 0;

			// get string length
			size_t dif = 0;
			while (sz[idx] != 0)
			{
				if (IsHighSurrogate(sz[idx]))
				{
					++idx; // go to low surrogate
					if (!IsLowSurrogate(sz[idx]))
						throw std::exception("rl::UTF16::DecodeString failed: Invalid input");
					++dif;
				}

				++idx;
			}
			const size_t lenVal = idx - dif;
			if (len)
				*len = lenVal;

			// create and initialize the shared pointer
			auto result = std::make_shared<char32_t[]>(lenVal + 1);
			memset(result.get(), 0, sizeof(char32_t) * (lenVal + 1));

			// decode the input
			idx = 0;
			dif = 0;
			while (sz[idx] != 0)
			{
				if (IsHighSurrogate(sz[idx]))
				{
					++dif;

					const auto cHi = sz[idx];
					++idx;
					const auto cLo = sz[idx];

					result[idx - dif] = Decode(cHi, cLo);
				}
				else
					result[idx - dif] = sz[idx];

				++idx;
			}

			return result;
		}
		
#endif // C++17

	}

}