#include "rl/dll/rlConsole/Font.h"
#include <memory>

bool rlConsole_Font_create(rlConsole_Font* pFont, size_t iCharCount, rlCon_char cFallback)
{
	if (iCharCount == 0)
		return false;

	pFont->pData = new rlConsole_Font_Char[iCharCount];
	if (!pFont->pData)
		return false;

	pFont->iCharCount = iCharCount;
	pFont->cFallback = cFallback;
	memset(pFont->pData, 0, iCharCount * sizeof(rlConsole_Font_Char));
	return true;
}

void rlConsole_Font_destroy(rlConsole_Font* pFont)
{
	delete[] pFont->pData;
	pFont->pData = nullptr;

	pFont->iCharCount = 0;
	pFont->cFallback = 0;
}
