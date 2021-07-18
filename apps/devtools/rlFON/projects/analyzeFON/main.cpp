/*

A small command line utility for retrieving metadata of a Microsoft FON bitmap font file

*/

#include "../../comps/parser.hpp"
#include "../../comps/consoleui.hpp"
#include <rl/console.hpp>
using Con = rl::Console;

#include <cstdio> // printf
#include <inttypes.h> // printf with 64 bit variables
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
	const char szAppName[] = "analyzeFON";
	Con::PushColor(FG_GRAY | BG_BLACK); // set to default colors
	Con::SetCodepage(1252); // set appropriate codepage to avoid corrupted output

	if (argc == 1 + 1 && (wcscmp(argv[1], L"/?") == 0 || wcscmp(argv[1], L"--help") == 0))
	{
		ShowSyntax();
		return ERROR_SUCCESS; // no error occured
	}

	if (argc != 1 + 1)
	{
		rl::WriteHelpHint(szAppName);
		return ERROR_BAD_ARGUMENTS;
	}

	wchar_t szPath[MAX_PATH + 1] = {};
	if (wcslen(argv[1]) <= MAX_PATH)
		wcscpy_s(szPath, argv[1]);
	else
	{
		rl::WriteError("Input path was too long");
		return ERROR_BAD_PATHNAME;
	}

	printf("Analyzing file \"%ls\"...\n\n", szPath);

	rl::MicrosoftFONParser parser;

	// try to parse file, on failure show error + exit
	if (!parser.parse(szPath))
	{
		uint8_t iError = parser.getParseError();
		rl::WriteFONError(iError);
		return -1;
	}

	// show warnings
	uint8_t iWarnings = parser.getWarnings();
	rl::WriteFONWarnings(iWarnings);


	// print version info (if any)
	rl::VERSIONINFO vi = {};
	if (parser.getVersionInfo(vi))
	{
		printf("Binary version info (\"\\\")\n{\n");


		printf("  FileVersion:\t\t%d,%d,%d,%d\n",
			uint16_t(vi.FileVersion >> 48),
			uint16_t(vi.FileVersion >> 32),
			uint16_t(vi.FileVersion >> 16),
			uint16_t(vi.FileVersion));

		printf("  ProductVersion:\t%d,%d,%d,%d\n",
			uint16_t(vi.ProductVersion >> 48),
			uint16_t(vi.ProductVersion >> 32),
			uint16_t(vi.ProductVersion >> 16),
			uint16_t(vi.ProductVersion));

		printf("  FileFlagsMask:\t0x%xL\n", vi.FileFlagsMask);

		// FileFlags
		{
			printf("  FileFlags:\t\t");
			const uint32_t iFF = vi.FileFlags;
			if (iFF == 0)
				printf("0x0L");
			else if (iFF == VS_FFI_FILEFLAGSMASK)
				printf("VS_FFI_FILEFLAGSMASK");
			else // custom combination
			{
#define PRINTSTR(s)						\
				if (iFF & s)			\
				{						\
					if (bFlag)			\
						printf(" | ");	\
					printf(#s);			\
					bFlag = true;		\
				}


				bool bFlag = false; // has a flag already been written? (for "|")

				PRINTSTR(VS_FF_DEBUG);
				PRINTSTR(VS_FF_PATCHED);
				PRINTSTR(VS_FF_PRERELEASE);
				PRINTSTR(VS_FF_PRIVATEBUILD);
				PRINTSTR(VS_FF_SPECIALBUILD);

#undef PRINTSTR
			}
			printf("\n");
		}

		// FileOS
		{
#define PRINTSTR(s) case s: printf(#s); break
			printf("  FileOS:\t\t");
			switch (vi.FileOS)
			{
				PRINTSTR(VOS_UNKNOWN);
				PRINTSTR(VOS_DOS);
				PRINTSTR(VOS_NT);
				PRINTSTR(VOS__WINDOWS16);
				PRINTSTR(VOS__WINDOWS32);
				PRINTSTR(VOS_DOS_WINDOWS16);
				PRINTSTR(VOS_DOS_WINDOWS32);
				PRINTSTR(VOS_NT_WINDOWS32);

			}
			printf("\n");
#undef PRINTSTR
		}

		printf("  FileType:\t\tVFT_FONT\n");
		printf("  FileSubtype:\t\tVFT2_FONT_RASTER\n");


		printf("}\n\n");



		// output version info strings
		for (auto& o : vi.LangStrings)
		{
			char szLangID[9];
			sprintf_s(szLangID, "%04X%04X", o.first.wLanguage, o.first.wCodePage);

			printf("String version info (\"\\StringFileInfo\\%s\")\n{\n", szLangID);
#define PRINTSTR(s, sTabs)	if(o.second.##s.length() != 1 || o.second.##s[0] != 0) \
								printf("  " #s ":" sTabs "%ls\n", o.second.##s.c_str())

			PRINTSTR(Comments, "\t\t");
			PRINTSTR(CompanyName, "\t\t");
			PRINTSTR(FileDescription, "\t");
			PRINTSTR(FileVersion, "\t\t");
			PRINTSTR(InternalName, "\t\t");
			PRINTSTR(LegalCopyright, "\t");
			PRINTSTR(LegalTrademarks, "\t");
			PRINTSTR(OriginalFilename, "\t");
			PRINTSTR(ProductName, "\t\t");
			PRINTSTR(ProductVersion, "\t");
			PRINTSTR(PrivateBuild, "\t\t");
			PRINTSTR(SpecialBuild, "\t\t");

#undef PRINTSTR
			printf("}\n\n");
		}
	}



	// print font resource info

	std::vector<rl::FONTDIRENTRY> oFontDir;
	parser.getFontDir(oFontDir);

	printf("Font data (\"\\FONT\") [showing raster fonts only]\nCount: %i\n{\n",
		(int)oFontDir.size());
	for (auto& o : oFontDir)
	{
		printf("  [%d]\t\"%s\"\tFNT version 0x%04x\tCharSet %d\t%3dx%d\t\t%dPt\n",
			o.fontOrdinal, o.sFaceName.c_str(), o.oHeader.dfVersion, o.oHeader.dfCharSet,
			o.oHeader.dfAvgWidth, o.oHeader.dfPixHeight, o.oHeader.dfPoints);
	}
	printf("}\n\n");

	return ERROR_SUCCESS;
}





void ShowSyntax()
{
	printf("\n"
		"usage: analyzeFON [/? | --help] <InputPath>\n"
		"\n"
		"  /? | --help	Show syntax\n"
		"  InputPath	Path of a .FON file\n"
		"\n"
	);
}