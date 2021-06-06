#include "consoleui.hpp"

#include "parser.hpp"

#include <rl/console.hpp>

#include <stdint.h>





namespace rl
{

	void WriteWarning(const char* szText)
	{
		Console::PushColor(FG_YELLOW);
		printf("Warning: ");
		Console::PopColor();
		printf(szText);
		printf("\n");
	}

	void WriteError(const char* szText)
	{
		Console::PushColor(FG_RED);
		printf("Error: ");
		Console::PopColor();
		printf(szText);
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


			/*

#define RL_FONPARSER_E_NOERROR				0 // no error occured
#define RL_FONPARSER_E_FILEERROR			1 // file doesn't exist or couldn't be read
#define RL_FONPARSER_E_NOFONTFILE			2 // file is no font
#define RL_FONPARSER_E_NORASTERFONTFILE		3 // file is a font, but no raster font
#define RL_FONPARSER_E_NOFONTRESOURCE		4 // no FONT resource found
#define RL_FONPARSER_E_UNKNOWNVERSION		5 // the FNT version was not 2

			*/

		default:
			rl::WriteError("Unknown error");
			break;
		}

		printf("\n\n");
	}

}