/*

A small command line utility for retrieving metadata of a Microsoft FNT bitmap font file

*/

#include "../../comps/parser.hpp"
#include "../../comps/consoleui.hpp"
#include <rl/console.hpp>
using Con = rl::Console;

#include <cstdio> // printf
#include <Shlwapi.h>
#include <Windows.h>

#pragma comment(lib, "Shlwapi.lib")





/// <summary>
/// Show the correct syntax for the application
/// </summary>
void ShowSyntax();



int wmain(int argc, wchar_t* argv[])
{
	SetConsoleTitleA("RobinLe's bitmap font analyzer");
	const char szAppName[] = "analyzeFNT";
	Con::PushColor(FG_GRAY | BG_BLACK); // set to default colors
	Con::SetCodepage(1252); // set appropriate codepage to avoid corrupted output

	if (argc == 1 + 1 && (wcscmp(argv[1], L"/?") == 0 || wcscmp(argv[1], L"--help") == 0))
	{
		ShowSyntax();
		return ERROR_SUCCESS; // no error occured
	}

	if (argc < 1 + 1)
	{
		rl::WriteHelpHint(szAppName);
		return ERROR_BAD_ARGUMENTS;
	}

	wchar_t szPath[MAX_PATH + 1] = {};
	int iPathArgs = rl::CmdGetPath(argc, argv, 0, 0, szPath);
	if (iPathArgs == 0)
	{
		rl::WriteHelpHint(szAppName);
		return ERROR_BAD_PATHNAME;
	}



	printf("Analyzing file \"%ls\"...\n\n", szPath);

	HANDLE hFile = CreateFileW(szPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL,
		NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		rl::WriteError("Couldn't open the file");
		return ERROR_FILE_NOT_FOUND;
	}

	LARGE_INTEGER liFileSize;
	GetFileSizeEx(hFile, &liFileSize);

	uint8_t* pData = new uint8_t[(size_t)liFileSize.QuadPart];
	DWORD dwRead = 0;
	BOOL b = ReadFile(hFile, pData, liFileSize.LowPart, &dwRead, NULL);
	CloseHandle(hFile);
	if (!b)
	{
		delete[] pData;
		return ERROR_FILE_CORRUPT;
	}
	rl::MicrosoftRasterFont font;
	font.create(pData, (size_t)liFileSize.QuadPart);
	delete[] pData;

	if (!font.hasData())
	{
		rl::WriteFONError(font.getError());
		return ERROR_FILE_CORRUPT;
	}
	rl::FONTHDR hdr = {};
	font.getHeader(hdr);

	// output font metadata
	printf(
		"  FaceName:\t\"%s\"\n"
		"  FNT version:\t0x%04x\n"
		"  CharSet:\t%d\n"
		"  Points:\t%d\n"
		"  \n"
		"  Height:\t%d\n",
		font.faceName(), hdr.dfVersion, hdr.dfCharSet, hdr.dfPoints, hdr.dfPixHeight);

	if (hdr.dfPixWidth > 0) // global width
		printf("  Width:\t%d\n", hdr.dfPixWidth);
	else // variable width
	{
		printf(
			"  Width avg.:\t%d\n"
			"  Width max:\t%d\n",
			hdr.dfAvgWidth, hdr.dfMaxWidth);
	}

	printf("\n\n");
	return ERROR_SUCCESS;
}





void ShowSyntax()
{
	printf("\n"
		"usage: analyzeFNT [/? | --help] <InputPath>\n"
		"\n"
		"  /? | --help	Show syntax\n"
		"  InputPath	Path of a .FNT file\n"
		"\n"
	);
}