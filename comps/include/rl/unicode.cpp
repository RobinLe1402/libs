#include "unicode.hpp"





namespace rl
{
	namespace Unicode
	{

		bool IsPrivateUse(char32_t ch)
		{
			return ((ch >= 0xE000 && ch <= 0xF8FF) /* Private Use Zone */ ||
				(ch >= 0xF0000 && ch < 0x110000) /* Private Use Planes */);
		}

		bool IsNoncharacter(char32_t ch)
		{
			return ((ch >= 0xFDD0 && ch <= 0xFDEF) /* noncharacter range in BMP */ ||
				(ch & 0xFFFF) >= 0xFFFE /* last two code points of each plane */);
		}

	}
}