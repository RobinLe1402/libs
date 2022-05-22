#include "include/CodePageToUnicode.hpp"

#include <Windows.h>

// translation tables for the special characters assigned to control characters in DOS code pages.
constexpr char32_t cDOS[] =
{
	// 0x00..0x0F
	U'\u0000', U'\u263A', U'\u263B', U'\u2665', U'\u2666', U'\u2663', U'\u2660', U'\u2022',
	U'\u25D8', U'\u25CB', U'\u25D9', U'\u2642', U'\u2640', U'\u266A', U'\u266B', U'\u263C',

	// 0x10..0x1F
	U'\u25BA', U'\u25C4', U'\u2195', U'\u203C', U'\u00B6', U'\u00A7', U'\u25AC', U'\u21A8',
	U'\u2191', U'\u2193', U'\u2192', U'\u2190', U'\u221F', U'\u2194', U'\u25B2', U'\u25BC'
};
constexpr char32_t cIBM[] =
{
	// 0x00..0x0F
	U'\u0000', U'\u263A', U'\u266A', U'\u266B', U'\u263C', U'\u2550', U'\u2551', U'\u256C',
	U'\u2563', U'\u2566', U'\u2560', U'\u2569', U'\u2557', U'\u2554', U'\u255A', U'\u255D',

	// 0x10..0x1F
	U'\u25BA', U'\u25C4', U'\u2195', U'\u203C', U'\u00B6', U'\u00A7', U'\u25AC', U'\u21A8',
	U'\u2191', U'\u2193', U'\u2192', U'\u2190', U'\u221F', U'\u2194', U'\u25B2', U'\u25BC'
};
// not listed: 0x7F is assigned U+2302

bool CodePageToUnicode(uint16_t iCodePage, uint16_t iCodePageID, char32_t& iDest)
{
	wchar_t sz[2] = {};
	if (!MultiByteToWideChar(iCodePage, MB_ERR_INVALID_CHARS, (char*)&iCodePageID, 1, sz, 2))
		return false;

	if (sz[0] <= 0x1F || sz[0] == 0x7F)
	{
		switch (iCodePage)
		{
			// DOS codepages --> special lookup tables
			// (invalid conversion for actual text, but this function provides conversion for
			// font files, so it's appropriate here)
		case 437:
		case 708:
		case 720:
		case 737:
		case 775:
		case 850:
		case 852:
		case 855:
		case 857:
		case 860:
		case 861:
		case 862:
		case 863:
		case 865:
		case 866:
		case 869:
			if (sz[0] == 0x7F)
				iDest = 0x2302;
			else // sz[0] <= 0x1F
				iDest = cDOS[sz[0]];
			return true;

		case 864:
			if (sz[0] == 0x7F)
				iDest = 0x2302;
			else // sz[0] <= 0x1F
				iDest = cIBM[sz[0]];
			return true;
		}
	}



	// UTF-16 decoding

	if ((sz[0] & 0xFC00) != 0xD800) // no surrogates
		iDest = sz[0];
	else // surrogates
	{
		uint32_t iTMP = 0;

		iTMP = sz[1] & 0x3FFF;
		iTMP |= uint32_t(sz[0] & 0x3FFF) << 10;
		iTMP += 0x10000;

		iDest = iTMP;
	}

	return true;
}