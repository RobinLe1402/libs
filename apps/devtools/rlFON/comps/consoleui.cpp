#include "consoleui.hpp"

#include "parser.hpp"

#include <rl/console.hpp>

#include <stdint.h>





namespace rl
{

	void WriteWarning(const char* szText, ...)
	{
		Console::PushColor(FG_YELLOW);
		printf("Warning: ");
		Console::PopColor();

		va_list args;
		va_start(args, szText);
		vprintf(szText, args);
		va_end(args);

		printf("\n");
	}

	void WriteError(const char* szText, ...)
	{
		Console::PushColor(FG_RED);
		printf("Error: ");
		Console::PopColor();

		va_list args;
		va_start(args, szText);
		vprintf(szText, args);
		va_end(args);

		printf("\n");
	}

	bool WriteWin32Error(DWORD dwError)
	{
		if (!dwError) // no error --> cancel
			return false;

		LPVOID lpMsgBuf = NULL;
		DWORD dwLen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf, 0, NULL);

		if (!dwLen) // unknown system error
		{
			Console::PushColor(FG_RED);
			printf("Unknown Win32 error\n");
			Console::PopColor();
			return true;
		}

		Console::PushColor(FG_RED);
		printf("Win32 error: ");
		Console::PopColor();
		printf("%s\n", (LPCSTR)lpMsgBuf);

		LocalFree(lpMsgBuf);
		return true;
	}

	void WriteHelpHint(const char* szExeFilename)
	{
		printf("Syntax error.\nFor help, type '");
		Console::PushColor(FG_YELLOW);
		printf(szExeFilename);
		printf(" /?");
		Console::PopColor();
		printf("' or '");
		Console::PushColor(FG_YELLOW);
		printf(szExeFilename);
		printf(" --help");
		Console::PopColor();
		printf("'.\n\n");
	}

	void WriteFONWarnings(uint8_t iWarningFlags)
	{
		if (iWarningFlags == 0)
			return;



		if (iWarningFlags & RL_FONPARSER_W_NOVERSIONINFO)
			WriteWarning("The file doesn't seem to have a VERSIONINFO");
		if (iWarningFlags & RL_FONPARSER_W_WRONGLANG)
			WriteWarning("The file has an incorrect VERSIONINFO translation table");
		if (iWarningFlags & RL_FONPARSER_W_INVALIDDATA)
			WriteWarning("The file includes invalid FONT data");

		printf("\n\n");
	}

	void WriteFONError(uint8_t iError)
	{
		switch (iError)
		{
		case RL_FONPARSER_E_FILEERROR:
			WriteError("Couldn't read the file");
			break;
		case RL_FONPARSER_E_NOFONTFILE:
			WriteError("File was no font file");
			break;
		case RL_FONPARSER_E_NORASTERFONTFILE:
			WriteError("File was no raster font file");
			break;
		case RL_FONPARSER_E_NOFONTRESOURCE:
			WriteError("The file contains no FONT resource");
			break;
		case RL_FONPARSER_E_NORASTERFONTRESOURCE:
			WriteError("The file contains no raster FONT resource");
			break;
		case RL_FONPARSER_E_UNKNOWNVERSION:
			WriteError("The file contains FNT data of an unknown version");
			break;


		default:
			rl::WriteError("Unknown error");
			break;
		}

		printf("\n\n");
	}

}