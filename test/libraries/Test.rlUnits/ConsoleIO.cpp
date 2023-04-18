#include "ConsoleIO.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <conio.h>
#include <memory>
#include <iostream>
#include <Windows.h>



Console Console::s_oInstance;

void Console::clear()
{
	system("CLS");
}

void Console::pause(bool bShowPrompt)
{
	if (bShowPrompt)
		write("Press any key to continue . . . ");

	auto iKey = _getch();

	// handle extended codes
	switch (iKey)
	{
	case 0:
	case 0xE0:
		(void)_getch();
		break;
	}

	if (bShowPrompt)
		write("\n");
}

void Console::bell()
{
	std::printf("\a");
}

bool Console::readUInt(uint64_t &result, uint64_t iMaxVal)
{
	setANSIEscapeSequences(false);

	std::cin.exceptions(std::ios::failbit | std::ios::badbit);

	bool bResult = true;
	
	try
	{
		uint64_t iVal = 0;
		std::cin >> iVal;

		if (iVal > iMaxVal)
			bResult = false;
		else
			result = iVal;
	}
	catch (...)
	{
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		bResult = false;
	}

	setANSIEscapeSequences(true);
	return bResult;
}

Console::Console()
{
	if (!setANSIEscapeSequences(true))
	{
		std::printf("Error enabling ANSI escape sequences for the console (Error: %u).\n",
			GetLastError());
		pause();
		exit(1);
	}
}

bool Console::setANSIEscapeSequences(bool bUse)
{
	//HANDLE hCon = GetStdHandle(STD_INPUT_HANDLE);
	//DWORD dwMode = 0;

	//if (!GetConsoleMode(hCon, &dwMode))
	//	return false; // failed to get console mode

	//if (bUse)
	//	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	//else
	//	dwMode &= ~ENABLE_VIRTUAL_TERMINAL_PROCESSING;

	//return SetConsoleMode(hCon, dwMode);

	return true;
}

char Console::readChar(bool *pExt, char *pExtCode)
{
	const char cKey = _getch();

	// handle extended codes
	switch (cKey)
	{
	case 0:
	case 0xE0:
	{
		if (pExt)
			*pExt = true;
		const char cExt = _getch();
		if (pExtCode)
			*pExtCode = cKey;
		return cExt;
	}

	default:
		if (pExt)
			*pExt = false;
		return cKey;
	}
}
