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

	int CmdGetPath(int argc, wchar_t* argv[], int startarg, size_t startindex,
		wchar_t(&dest)[MAX_PATH + 1])
	{
		startarg++;

		const wchar_t* sz = argv[startarg];
		size_t len = wcslen(sz);

		if (startarg >= argc || len < startindex + 1)
			return 0;

		sz += startindex;
		len -= startindex;

		// first parameter longer than MAX_PATH --> special treatment
		if (len - startindex > MAX_PATH)
		{
			// path is too long in any case
			if (len - 2 > MAX_PATH)
				return 0;

			if (argv[startarg][startindex] != L'"' || argv[startarg][len - 1] != L'"')
				return 0;

			memcpy_s(dest, sizeof(wchar_t) * (MAX_PATH + 1), argv[startarg] + startindex,
				sizeof(wchar_t) * (len - 2));
			dest[len - 2] = 0; // set terminating zero
			return 1;
		}



		// path starts with quote --> path must end with quote
		if (sz[startindex] == L'"')
		{
			wchar_t szTMP[MAX_PATH + 1] = {};

			sz += 1;
			if (wcslen(sz) == 0) // first parameter can't be single quote
				return 0;

			int i = 0;
			bool bEndFound = false;
			while (true)
			{
				if (sz[wcslen(sz) - 1] == L'"')
				{
					if (MAX_PATH - wcslen(szTMP) < wcslen(sz))
						return 0;
					bEndFound = true;
					wcscat_s(szTMP, L" ");
					wcsncat_s(szTMP, MAX_PATH + 1, sz, wcslen(sz) - 1);
					break;
				}
				else
				{
					if (MAX_PATH - wcslen(szTMP) < wcslen(sz) + 1)
						return 0;
					wcscat_s(szTMP, L" ");
					wcscat_s(szTMP, sz);
				}
				i++;
				if (i + 1 == argc)
					break;

				sz = argv[i + 1];
			}

			if (!bEndFound)
				return 0;

			wcscpy_s(dest, MAX_PATH + 1, szTMP);
			return i + 1;
		}



		// path doesn't start with quote --> single argument path
		wcscpy_s(dest, sz);
		return 1;
	}

}