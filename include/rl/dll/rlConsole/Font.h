#pragma once
#ifndef ROBINLE_CONSOLE_FONT
#define ROBINLE_CONSOLE_FONT





#include "Base.h"



typedef uint8_t rlConsole_Font_Char_Row;

#define rlConsole_Font_Char_Width 8
#define rlConsole_Font_Char_Height 16

typedef struct tag_rlConsole_Font_Char
{
	rlCon_char c;
	rlConsole_Font_Char_Row iData[rlConsole_Font_Char_Height];
} rlConsole_Font_Char;

typedef struct tag_rlConsole_Font
{
	size_t iCharCount;
	rlConsole_Font_Char* pData;
	rlCon_char cFallback;
	uint32_t iUnused;
} rlConsole_Font;



RLCONSOLE_API bool __stdcall rlConsole_Font_create(rlConsole_Font* pFont, size_t iCharCount,
	rlCon_char cFallback);
RLCONSOLE_API void __stdcall rlConsole_Font_destroy(rlConsole_Font* pFont);





#endif // ROBINLE_CONSOLE_FONT