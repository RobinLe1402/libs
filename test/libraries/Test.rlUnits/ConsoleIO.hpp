#pragma once
#ifndef ROBINLE_TEST_RLUNITS__CONSOLEIO
#define ROBINLE_TEST_RLUNITS__CONSOLEIO





#include <cstdint>
#include <cstdio>



class Console final
{
public: // static methods

	static auto &Instance() { return s_oInstance; }


private: // static methods


private: // static variables

	static Console s_oInstance;


public: // methods

	template <typename... T>
	void write(const char *szFormat, T... args)
	{
		std::printf(szFormat, args...);
	}

	void clear();

	void pause(bool bShowPrompt = true);

	void bell();

	bool readUInt(uint64_t &result, uint64_t iMaxVal = UINT64_MAX);


private: // methods

	Console();

	bool setANSIEscapeSequences(bool bUse);

	char readChar(bool *bExt, char *pExtCode);

};





#endif // ROBINLE_TEST_RLUNITS__CONSOLEIO