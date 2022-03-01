/*

A small command line utility for creating a preview image of all characters in a font from a
Microsoft FON file (by filename and "font ordinal" = font ID)

*/

#include "../!include/parser.hpp"
#include "../!include/consoleui.hpp"

#include <rl/console.hpp>
using Con = rl::Console;
#include <rl/tools.gdiplus.hpp>

#include <Shlwapi.h>

#pragma comment(lib, "Gdiplus.lib")
#pragma comment(lib, "Shlwapi.lib")





/// <summary>
/// Show the correct syntax for the application
/// </summary>
void ShowSyntax();

/// <summary>
/// Get the CLSID of a certain image encoder
/// </summary>
/// <param name="ID">= image encoding ID, formatted like this: "image/bmp"</param>
/// <returns>Did the function succeed?</returns>
bool GetEncoderCLSID(const wchar_t* ID, CLSID& dest);

int CreatePreview(const rl::MicrosoftRasterFont& font, const wchar_t* szBMP);



int wmain(int argc, wchar_t* argv[])
{
	SetConsoleTitleA("RobinLe's bitmap font preview image creator");
	const char szAppName[] = "previewFON";
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
		Set, // wFontOrdinal must be used
		First, // "?" --> first in file
		Default, // "~" --> width >= 8 if available
		All // "*" --> all
	} oOrdinalMode;

	WORD wFontOrdinal = 0;
	if (wcscmp(argv[2], L"?") == 0) // ordinal parameter "?" --> choose first one in file
		oOrdinalMode = FontOrdinalMode::First;
	else if (wcscmp(argv[2], L"~") == 0) // ordinal parameter "~" --> default font (width >= 8)
		oOrdinalMode = FontOrdinalMode::Default;
	else if (wcscmp(argv[2], L"*") == 0) // ordinal parameter "*" --> all fonts
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
		oOrdinalMode = FontOrdinalMode::Set;
	}



	// get BMP output path
	wchar_t szPathBMP[MAX_PATH + 1] = {};
	if (wcslen(argv[3]) <= MAX_PATH)
		wcscpy_s(szPathBMP, argv[3]);
	else
	{
		rl::WriteError("Output path was too long");
		return ERROR_BAD_PATHNAME;
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

	// if not determined, set font ordinal to first in file
	switch (oOrdinalMode)
	{
	case FontOrdinalMode::First:
	{
		std::vector<rl::FONTDIRENTRY> oFontDir;
		parser.getFontDir(oFontDir);
		wFontOrdinal = oFontDir[0].fontOrdinal;
	}
	break;

	case FontOrdinalMode::Default:
	{
		std::vector<rl::FONTDIRENTRY> oFontDir;
		parser.getFontDir(oFontDir);

		bool bFound = false;
		size_t iIndex = 0;
		uint16_t iWidth = 0;
		uint16_t iHeight = 0;

		// search for perfect match
		for (size_t i = 0; i < oFontDir.size(); i++)
		{
			rl::FONTHDR& hdr = oFontDir[i].oHeader;
			if (hdr.dfPixHeight >= 8 && hdr.dfMaxWidth == 8)
			{
				iIndex = i;
				bFound = true;
				break;
			}
		}

		// search for closest wider, height >= 8
		iWidth = 0xFFFF;
		if (!bFound)
		{
			for (size_t i = 0; i < oFontDir.size(); i++)
			{
				rl::FONTHDR& hdr = oFontDir[i].oHeader;
				if (hdr.dfPixHeight >= 8 && hdr.dfMaxWidth > 8 && hdr.dfMaxWidth < iWidth)
				{
					iIndex = i;
					bFound = true;
					iWidth = hdr.dfMaxWidth;
				}
			}
		}

		// search for closest narrower, height >= 8
		iWidth = 0;
		if (!bFound)
		{
			for (size_t i = 0; i < oFontDir.size(); i++)
			{
				rl::FONTHDR& hdr = oFontDir[i].oHeader;
				if (hdr.dfPixHeight >= 8 && hdr.dfMaxWidth > iWidth)
				{
					iIndex = i;
					bFound = true;
					iWidth = hdr.dfMaxWidth;
				}
			}
		}

		// search for closest, height < 8
		iWidth = 0;
		if (!bFound)
		{
			// search for height >= 8
			for (uint16_t iX = 7; iX > 0; iX--)
			{
				for (size_t i = 0; i < oFontDir.size(); i++)
				{
					rl::FONTHDR& hdr = oFontDir[i].oHeader;
					if (hdr.dfMaxWidth == iX && hdr.dfPixHeight >= 8 && hdr.dfPixHeight < iHeight)
					{
						iIndex = i;
						bFound = true;
						iHeight = hdr.dfPixHeight;
					}
				}
			}

			// search for height < 8
			for (uint16_t iX = 7; iX > 0; iX--)
			{
				for (size_t i = 0; i < oFontDir.size(); i++)
				{
					rl::FONTHDR& hdr = oFontDir[i].oHeader;
					if (hdr.dfMaxWidth == iX && hdr.dfPixHeight < 8 && hdr.dfPixHeight > iHeight)
					{
						iIndex = i;
						bFound = true;
						iHeight = hdr.dfPixHeight;
					}
				}
			}
		}

		wFontOrdinal = oFontDir[iIndex].fontOrdinal;
	}
	break;
	}

	// try to get font data, on failure show error + exit
	rl::MicrosoftRasterFont font;

	if (oOrdinalMode == FontOrdinalMode::All)
	{
		PathAddBackslashW(szPathBMP);
		const wchar_t* const szSourceFilename = PathFindFileNameW(szPathFON);

		// destination path length: + "\", + szSourceFilename "-XXXXX" (XXXXX = padded resource ID)
		if (MAX_PATH - wcslen(szPathBMP) < 1 + wcslen(szSourceFilename) + 1 + iResIDPadding)
		{
			rl::WriteError("Output path was too long.");
			return ERROR_FILENAME_EXCED_RANGE;
		}

		std::vector<rl::FONTDIRENTRY> oFontDir;
		parser.getFontDir(oFontDir);
		wcscat_s(szPathBMP, szSourceFilename);
		if (_wcsicmp(PathFindExtensionW(szPathBMP), L".fon") == 0)
			PathRemoveExtensionW(szPathBMP);
		wcscat_s(szPathBMP, L"-00000.bmp"); // dependent on iResIDPadding
		const size_t iResIDOffset = wcslen(szPathBMP) - 9;

		wchar_t szPaddedOrdinal[iResIDPadding + 1] = {};

		size_t iSuccessCount = 0;
		for (auto& o : oFontDir)
		{
			szPaddedOrdinal[0] = 0;
			swprintf_s(szPaddedOrdinal, L"%05d", o.fontOrdinal); // dependent on iResIDPadding
			memcpy_s(szPathBMP + iResIDOffset, iResIDPadding * sizeof(wchar_t), szPaddedOrdinal,
				iResIDPadding * sizeof(wchar_t));

			rl::MicrosoftRasterFont font;
			parser.getFont(o.fontOrdinal, font);

			int i = CreatePreview(font, szPathBMP);

			if (i != ERROR_SUCCESS)
				rl::WriteError("Couldn't save bitmap file \"%ls\"", szPathBMP);
			else
			{
				printf("Successfully saved bitmap font preview to \"%ls\"\n", szPathBMP);
				iSuccessCount++;
			}
		}
		printf("\n"
			"%llu/%llu successfully saved\n\n\n", (uint64_t)iSuccessCount, (uint64_t)oFontDir.size());

		if (iSuccessCount == 0)
			return -1;
	}
	else
	{
		if (!parser.getFont(wFontOrdinal, font))
		{
			rl::WriteError("Couldn't load FONT resource with ID %d", wFontOrdinal);
			return -1;
		}

		int iResult = CreatePreview(font, szPathBMP);
		if (iResult != ERROR_SUCCESS)
		{
			rl::WriteError("Couldn't write bitmap file \"%ls\"", szPathBMP);
			return iResult;
		}
		else
			printf("Successfully saved bitmap font preview to \"%ls\"\n\n\n", szPathBMP);
	}

	return ERROR_SUCCESS;
}


void ShowSyntax()
{
	printf("\n"
		"usage: previewFON [/? | --help] <InputPath> [FontOrdinal | ? | *] <OutputPath>\n"
		"\n"
		"  /? | --help	Show syntax\n"
		"\n"
		"Paths\n"
		"  InputPath	Path of a .FON file\n"
		"  OutputPath	Path for the output .BMP file\n"
		"\n"
		"Font resource identifier\n"
		"  FontOrdinal\tUnique integer ID of the FONT resource in the file (get using analyzeFON)\n"
		"  ?\t\tFirst FONT resource in the file will be used\n"
		"  ~\t\t\"Default\" FONT resource will be used.\n"
		"  \t\tThe \"default\" FONT resource is determined by it's height and maximum width:\n"
		"  \t\tif there is a font resource with a maximum width and height >= 8, this resource "
		"will be chosen.\n"
		"  \t\tIf not, the FONT resource with the closest width and height is chosen (Bigger "
		"preferred).\n"
		"\n"
	);
}

bool GetEncoderCLSID(const wchar_t* ID, CLSID& dest)
{
	// note: malloc() and free() must be used - when using new[] and delete[], delete[] fails

	Gdiplus::ImageCodecInfo* pCodecInfo = nullptr;
	UINT iEncCount = 0;
	UINT iEncSize = 0;
	Gdiplus::GetImageEncodersSize(&iEncCount, &iEncSize);
	if (iEncCount == 0)
		return false;

	pCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(iEncSize));
	if (pCodecInfo == nullptr)
		return false;
	Gdiplus::GetImageEncoders(iEncCount, iEncSize, pCodecInfo);

	for (uint64_t i = 0; i < iEncCount; i++)
	{
#pragma warning(suppress: 6385) // "Reading invalid data" false positive
		if (wcscmp(pCodecInfo[i].MimeType, ID) == 0)
		{
			dest = pCodecInfo[i].Clsid;
			free(pCodecInfo);
			return true;
		}
	}

	free(pCodecInfo);
	return false;
}

int CreatePreview(const rl::MicrosoftRasterFont& font, const wchar_t* szBMP)
{
	// get the font's header (for maximum width)
	rl::FONTHDR hdr = {};
	font.getHeader(hdr);

	const int iSeperatorSize = 1; // pixels between characters and outside padding
	const int iWidth = hdr.dfMaxWidth * 16 + iSeperatorSize * 17;
	const int iHeight = hdr.dfPixHeight * 16 + iSeperatorSize * 17;

	rl::GDIPlus gdip;
	Gdiplus::Bitmap* bmp = new Gdiplus::Bitmap(iWidth, iHeight, PixelFormat1bppIndexed);

	// fill image with white color
	Gdiplus::Graphics* mem = Gdiplus::Graphics::FromImage(bmp);
	Gdiplus::SolidBrush brushBG(Gdiplus::Color::White);
	mem->FillRectangle(&brushBG, 0, 0, bmp->GetWidth(), bmp->GetHeight());

	for (uint16_t i = 0; i < 256; i++)
	{
		if (!font.containsChar((uint8_t)i))
			continue;

		const uint8_t iCol = i % 16;
		const uint8_t iRow = i / 16;
		const int iXStart = iCol * hdr.dfMaxWidth + (iCol + 1) * iSeperatorSize;
		const int iYStart = iRow * hdr.dfPixHeight + (iRow + 1) * iSeperatorSize;
		rl::MicrosoftRasterChar ch;
		font.getChar((uint8_t)i, ch);

		for (uint32_t iX = 0; iX < ch.getWidth(); iX++)
		{
			for (uint32_t iY = 0; iY < ch.getHeight(); iY++)
			{
				Gdiplus::Color col;
				col.SetValue(ch.getPixel(iX, iY) ? 0xFF000000 : 0xFFFFFFFF);
				bmp->SetPixel(iXStart + iX, iYStart + iY, col);
			}
		}
	}

	int iResult = ERROR_SUCCESS;
	CLSID enc;
	GetEncoderCLSID(L"image/bmp", enc);
	if (bmp->Save(szBMP, &enc) != Gdiplus::Ok)
		iResult = -1;

	delete bmp;

	return iResult;
}