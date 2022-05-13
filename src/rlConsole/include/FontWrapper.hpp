#pragma once
#ifndef ROBINLE_CONSOLE_FONTWRAPPER
#define ROBINLE_CONSOLE_FONTWRAPPER





#include "rl/dll/rlConsole/Font.h"
#include <map>

struct WrappedCharData
{
	rlConsole_Font_Char_Row iData[rlConsole_Font_Char_Height];
};
constexpr size_t WrappedCharDataSize = sizeof(rlConsole_Font_Char_Row) * rlConsole_Font_Char_Height;

class FontWrapper
{
public: // operators

	operator bool() const;


public: // methods

	FontWrapper(rlConsole_Font& oFont);
	virtual ~FontWrapper() = default;

	const WrappedCharData* getChar(rlCon_char c) const;

private: // variables

	std::map<rlCon_char, WrappedCharData> m_oChars;
	rlCon_char m_cFallback;

};





#endif // ROBINLE_CONSOLE_FONTWRAPPER