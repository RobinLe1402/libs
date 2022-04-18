#include "include/CodePageToUnicode.hpp"

#include <Windows.h>

bool CodePageToUnicode(uint16_t iCodePage, uint16_t iCodePageID, char32_t& iDest)
{
	wchar_t sz[2] = {};
	if (!MultiByteToWideChar(iCodePage, MB_ERR_INVALID_CHARS, (char*)&iCodePageID, 1, sz, 2))
		return false;



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