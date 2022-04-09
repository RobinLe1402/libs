/*

A small command line utility for extracting FONT resources (FNT files) from Microsoft FON bitmap
font files

*/

#include "../!include/parser.hpp"
#include "../!include/consoleui.hpp"
#include "rl/console.hpp"
using Con = rl::Console;

#include <cstdio> // printf
#include <Shlwapi.h>
#include <Windows.h>

#pragma comment(lib, "Shlwapi.lib")





/// <summary>
/// Show the correct syntax for the application
/// </summary>
void ShowSyntax();

/// <summary>
/// Save raw FNT data to a .FNT file
/// </summary>
bool SaveFNT(const uint8_t* pData, size_t size, const wchar_t* szFilepath);



int wmain(int argc, wchar_t* argv[])
{
	SetConsoleTitleA("RobinLe's bitmap font preview image creator");
	const char szAppName[] = "extractFNT";
	Con::PushColor(FG_GRAY | BG_BLACK); // set to default colors
	Con::SetCodepage(1252); // set appropriate codepage to avoid corrupted output

	if (argc == 2 && (wcscmp(argv[1], L"/?") == 0 || wcscmp(argv[1], L"--help") == 0))
	{
		ShowSyntax();
		return ERROR_SUCCESS; // no error occured
	}

	if (argc != 3 + 1)
	{
		rl::WriteHelpHint(szAppName);
		return ERROR_BAD_ARGUMENTS;
	}



	const uint8_t iResIDPadding = 5; // --> max "65535". constant can't be larger than five digits.


	// get FON input path
	wchar_t szPathFON[MAX_PATH + 1] = {};
	if (wcslen(argv[1]) <= MAX_PATH)
		wcscpy_s(szPathFON, argv[1]);
	else
	{
		rl::WriteError("Input path was too long");
		return ERROR_BAD_PATHNAME;
	}



	// get font ordinal
	
	enum class FontOrdinalMode
	{
		Single, // extract FONT <wFontOrdinal> only
		All // "*" --> extract all
	} oOrdinalMode;


	WORD wFontOrdinal = 0;
	if (wcscmp(argv[2], L"*") == 0) // ordinal parameter "*" --> extract all
		oOrdinalMode = FontOrdinalMode::All;
	else
	{
		const wchar_t* szOrdinal = argv[2];

		// check if valid integer
		for (size_t i = 0; i < wcslen(szOrdinal); i++)
		{
			if (szOrdinal[i] < L'0' || szOrdinal[i] > L'9')
			{
				rl::WriteHelpHint(szAppName);
				return ERROR_INVALID_PARAMETER;
			}
		}

		wFontOrdinal = (WORD)wcstoul(argv[2], nullptr, 10);
		oOrdinalMode = FontOrdinalMode::Single;
	}



	// get output path
	wchar_t szPathOutput[MAX_PATH + 1] = {};
	if (wcslen(argv[3]) <= MAX_PATH)
		wcscpy_s(szPathOutput, argv[3]);
	else
	{
		rl::WriteError("Output path was too long");
			return ERROR_BAD_PATHNAME;
	}



	const wchar_t* const szSourceFilename = PathFindFileNameW(szPathFON);
	if (oOrdinalMode == FontOrdinalMode::All)
	{
		PathAddBackslashW(szPathOutput);
		DWORD dwAttribs = GetFileAttributesW(szPathOutput);
		if (dwAttribs == INVALID_FILE_ATTRIBUTES)
		{
			rl::WriteError("Couldn't access path \"%ls\".", szPathOutput);
			return GetLastError();
		}
		if (!(dwAttribs & FILE_ATTRIBUTE_DIRECTORY))
		{
			rl::WriteError("Path \"%ls\" is no directory", szPathOutput);
			return ERROR_BAD_PATHNAME;
		}

		// destination path length: + "\", + szSourceFilename "-XXXXX" (XXXXX = padded resource ID)
		if (MAX_PATH - wcslen(szPathOutput) < 1 + wcslen(szSourceFilename) + 1 + iResIDPadding)
		{
			rl::WriteError("Output path was too long.");
			return ERROR_FILENAME_EXCED_RANGE;
		}
	}


	rl::MicrosoftFONParser parser;

	// try to parse file, on failure show error + exit
	if (!parser.parse(szPathFON))
	{
		uint8_t iError = parser.getParseError();
		rl::WriteFONError(iError);
		return -1;
	}

	// show warnings
	uint8_t iWarnings = parser.getWarnings();
	rl::WriteFONWarnings(iWarnings);


	const uint8_t* pData;
	size_t size;
	rl::MicrosoftRasterFont font;
	switch (oOrdinalMode)
	{
	case FontOrdinalMode::Single:
	{
		printf("Extracting FONT resource #%d from \"%ls\"...\n\n", wFontOrdinal, szPathFON);
		if (!parser.getFont(wFontOrdinal, font))
		{
			rl::WriteError("The font ordinal was invalid.");
			return ERROR_INVALID_PARAMETER;
		}
		font.getData(pData, size);
		if (!SaveFNT(pData, size, szPathOutput))
		{
			rl::WriteError("Couldn't write file \"%ls\"", szPathOutput);
			return -1;
		}
		printf("Successfully created file \"%ls\"\n", szPathOutput);
	}
	break;
	case FontOrdinalMode::All:
	{
		printf("Extracting FNT data from \"%ls\"...\n\n", szPathFON);

		std::vector<rl::FONTDIRENTRY> oFontDir;
		parser.getFontDir(oFontDir);
		wcscat_s(szPathOutput, szSourceFilename);
		if (_wcsicmp(PathFindExtensionW(szPathOutput), L".fon") == 0)
			PathRemoveExtensionW(szPathOutput);
		wcscat_s(szPathOutput, L"-00000.fnt"); // dependent on iResIDPadding
		const size_t iResIDOffset = wcslen(szPathOutput) - 9;

		wchar_t szPaddedOrdinal[iResIDPadding + 1] = {};

		size_t iSuccessCount = 0;
		for (auto& o : oFontDir)
		{
			szPaddedOrdinal[0] = 0;
			swprintf_s(szPaddedOrdinal, L"%05d", o.fontOrdinal); // dependent on iResIDPadding
			memcpy_s(szPathOutput + iResIDOffset, iResIDPadding * sizeof(wchar_t), szPaddedOrdinal,
				iResIDPadding * sizeof(wchar_t));

			parser.getFont(o.fontOrdinal, font);
			font.getData(pData, size);
			if (!SaveFNT(pData, size, szPathOutput))
				rl::WriteError("Couldn't write file \"%ls\"", szPathOutput);
			else
			{
				printf("Successfully created file \"%ls\"\n", szPathOutput);
				iSuccessCount++;
			}
		}
		printf("\n"
			"%llu/%llu successfully saved\n", (uint64_t)iSuccessCount, (uint64_t)oFontDir.size());

		if (iSuccessCount == 0)
		{
			printf("\n");
			return -1;
		}
	}
	break;
	}

	printf("\n");
	return ERROR_SUCCESS;
}





void ShowSyntax()
{
	printf("\n"
		"usage: extractFNT [/? | --help] <InputPath> [FontOrdinal | *] <OutputPath>\n"
		"\n"
		"  /? | --help\tShow syntax\n"
		"\n"
		"Paths\n"
		"  InputPath\tPath of a .FON file\n"
		"  OutputPath\tWhen using a FontOrdinal:\tPath for the output .FNT file\n"
		"  \t\tWhen using \"*\":\t\t\tOutput folder\n"
		"\n"
		"Font resource identifier\n"
		"  FontOrdinal\tUnique integer ID of the FONT resource in the file (get using analyzeFON)\n"
		"  *\t\tExtract all FONT resources, with the file naming scheme "
		"\"<FonFilename>-<PaddedFontOrdinal>.fon\"\n"
		"  \t\tExample: File \"app850.fon\", FONT #1: \"app850-00001.fnt\"\n"
		"\n"
	);
}

bool SaveFNT(const uint8_t* pData, size_t size, const wchar_t* szFilepath)
{
	HANDLE hFile = CreateFileW(szFilepath, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	DWORD dwWritten = 0;
	BOOL b = WriteFile(hFile, pData, (DWORD)size, &dwWritten, NULL);
	if (!b)
	{
		CloseHandle(hFile);
		return false;
	}

	CloseHandle(hFile);
	return true;
}