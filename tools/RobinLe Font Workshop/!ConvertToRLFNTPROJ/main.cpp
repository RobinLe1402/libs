#include "../!Include/rlFNTPROJ.hpp"
namespace rlFont = rl::rlFNTPROJ;

// RobinLe
#include <rl/lib/RasterFontReader.hpp>
#include <rl/commandline.hpp>

// STL
#include <array>
#include <filesystem>
#include <string>
#include <string_view>

// Win32
#include <Windows.h>

using namespace std::string_literals;





void PrintUsage(const wchar_t *argv0);

bool GetCodepage(const wchar_t *argv, uint16_t& iDest);

std::wstring StringToWString(const char *szSrc);

uint64_t FontSignature(uint16_t iWeight, bool bItalic, bool bUnderline, bool bStrikethrough);

rlFont::TypefaceClassification LibFontTypeToProjectFontType(rl::RasterFontReader::FontType eType);

const wchar_t *WeightDisplayName(unsigned iWeight);

void GeneralizeSettings(rlFont::Project &oProject);



int wmain(int argc, wchar_t *argv[])
{
	if (argc < 2) // no additional argument --> error
	{
		PrintUsage(argv[0]);
		return 1;
	}

	auto &cmd = rl::Commandline::Instance();


	// parse [O]utput argument
	auto it = cmd.findNamedValue(L"O", false);
	const bool bOutputArg = it != cmd.end();
	if (!bOutputArg)
	{
		it = cmd.find(L"O", false);
		if (it != cmd.end())
		{
			printf("Error: Invalid \"O\" parameter.\n");
			return 1;
		}
	}
	std::wstring sOutputFile;
	if (bOutputArg)
		sOutputFile = it->value();

	// parse fallback [C]ode[P]age argument
	it = cmd.findNamedValue(L"CP", false);
	uint16_t iFallbackCP = 0; // Fallback Codepage. 0 if not given.
	const bool bFallbackCPArg = it != cmd.end();
	if (bFallbackCPArg)
	{
		try
		{
			iFallbackCP = std::stoi(it->value());
		}
		catch (...)
		{
			printf("Error: Fallback codepage was no number.\n");
			return 1;
		}

		if (!IsValidCodePage(iFallbackCP))
		{
			printf("Error: Invalid fallback codepage.\n");
			return 1;
		}
	}
	else
	{
		it = cmd.find(L"CP", false);
		if (it != cmd.end())
			printf("Warning: Invalid \"CP\" parameter; will be ignored.\n");
	}







	// ANALYZE INPUT FILE ==========================================================================


	constexpr wchar_t szExt_CPI[] = L".CPI";
	constexpr wchar_t szExt_FON[] = L".FON";
	constexpr wchar_t szExt_FNT[] = L".FNT";

	// list of all file extensions supported by the RasterFontReader library
	const std::array oLibExts =
	{
		szExt_CPI,
		szExt_FON,
		szExt_FNT
	};

	std::wstring_view sInputFile = argv[1];


	// check if input file exists
	const auto oInputPath = std::filesystem::path(sInputFile);
	const auto sInputFilename = oInputPath.filename().wstring();
	const auto oInputFileStatus = std::filesystem::status(oInputPath);
	if (!std::filesystem::status_known(oInputFileStatus) ||
		oInputFileStatus.type() != std::filesystem::file_type::regular)
	{
		printf("Error: \"%ls\" is no file.\n", sInputFile.data());
		return 1;
	}

	// extract input file extension, set it to uppercase only
	std::wstring sInputExt = oInputPath.extension().wstring();
	{
		std::locale loc;
		for (size_t i = 0; i < sInputExt.length(); ++i)
		{
			sInputExt[i] = std::toupper(sInputExt[i], loc);
		}
	}

	// check if input file extension is amongst the file extensions supported by the
	// RasterFontReader library
	if (std::find(oLibExts.begin(), oLibExts.end(), sInputExt) == oLibExts.end())
	{
		printf("Error: Unsupported file extension \"%ls\".\n"
			"The following file extensions are supported:\n", sInputExt.c_str());

		for (const wchar_t *sz : oLibExts)
		{
			printf("  \"%ls\"\n", sz);
		}

		return 1;
	}





	// PARSE THE INPUT FILE ========================================================================

	std::vector<rl::RasterFontReader::RasterFontFace> oInputFonts;

	if (sInputExt == szExt_FNT)
	{
		rl::RasterFontReader::RasterFontFace oInputFont;
		const auto eResult = oInputFont.loadFromFile_FNT(sInputFile.data(), iFallbackCP);
		using res = rl::RasterFontReader::LoadResult_FNT;

		if (eResult != res::Success)
		{
			printf("Error: ");
			switch (eResult)
			{
			case res::FileNotOpened:
				printf("Couldn't open input file for reading.");
				break;
			case res::UnexpectedEOF:
				printf("Unexpected EOF in input file.");
				break;
			case res::InternalError:
				printf("Internal error while reading input file.");
				break;
			case res::VectorFont:
				printf("Input file was a vector font instead of a raster font.");
				break;
			case res::UnsupportedVersion:
				printf("The FNT version of the input file is not supported.");
				break;
			case res::UnknownCharSet:
				printf("The input file uses an unknown character set.");
				break;
			case res::MultiColor:
				printf("The input file was not 2-colors-only.");
				break;
			default:
				printf("<Unknown error>");
				break;
			}
			printf("\n");

			return 1;
		}

		oInputFonts.push_back(std::move(oInputFont));
	}
	else if (sInputExt == szExt_CPI || sInputExt == szExt_FON)
	{
		rl::RasterFontReader::RasterFontFaceCollection oFileFonts;
		if (sInputExt == szExt_CPI)
		{
			const auto eResult = oFileFonts.loadFromFile_CPI(sInputFile.data());
			using res = rl::RasterFontReader::LoadResult_CPI;

			if (eResult != res::Success)
			{
				printf("Error: ");
				switch (eResult)
				{
				case res::FileNotOpened:
					printf("Couldn't open input file for reading.");
					break;
				case res::UnexpectedEOF:
					printf("Unexpected EOF in input file.");
					break;
				case res::InternalError:
					printf("Internal error while reading input file.");
					break;
				case res::UnexpectedValue:
					printf("Unexpected value found in input file.");
					break;
				default:
					printf("<Unknown error>");
					break;
				}
				printf("\n");

				return 1;
			}
		}
		else if (sInputExt == szExt_FON)
		{
			const auto eResult = oFileFonts.loadFromFile_FON(sInputFile.data(), iFallbackCP);
			using res = rl::RasterFontReader::LoadResult_FON;

			if (eResult != res::Success)
			{
				printf("Error: ");
				switch (eResult)
				{
				case res::FileNotOpened:
					printf("Couldn't open input file for reading.");
					break;
				case res::UnexpectedEOF:
					printf("Unexpected EOF in input file.");
					break;
				case res::InternalError:
					printf("Internal error while reading input file.");
					break;
				default:
					printf("<Unknown error>");
					break;
				}
				printf("\n");

				return 1;
			}
		}

		oInputFonts.append_range(oFileFonts.fontfaces());
	}

	if (oInputFonts.size() == 0)
	{
		printf("Error: Input file could be read but didn't contain any valid fonts.\n");
		if ((sInputExt == szExt_FNT || sInputExt == szExt_FON) && !bFallbackCPArg)
			printf("Consider adding a /CP:[Codepage] parameter to the call.\n");
		return 1;
	}

	printf("Successfully read input file.\n");





	// CONVERT TO FONT PROJECT =====================================================================

	// project

	rlFont::Project oProject;
	rlFont::ProjectMeta oProjectMeta = oProject.getMetadata();
	oProjectMeta.sComments = std::format(L"Created from file \"{}\" via ConvertToRLFNTPROJ",
		sInputFilename);
	oProject.setMetadata(oProjectMeta);



	// font family

	auto &oFontFamily = oProject.getFontFamily();
	rlFont::FontFamilyMeta oFontFamilyMeta = oFontFamily.getMetdata();

	// check font name(s)
	oFontFamilyMeta.sName = L"";
	bool bSingleFaceName = true;
	for (const auto &oFont : oInputFonts)
	{
		const auto &sSrcName = oFont.meta().sFaceName;
		std::wstring sSrcNameW = StringToWString(sSrcName.c_str());

		if (sSrcNameW.empty())
			continue;
		else if (oFontFamilyMeta.sName.empty())
			oFontFamilyMeta.sName = sSrcNameW;
		else if (oFontFamilyMeta.sName != sSrcNameW)
		{
			oFontFamilyMeta.sName = sInputFilename;
			bSingleFaceName = false;
			break;
		}
	}

	// check font type(s)
	rlFont::TypefaceClassification eType =
		LibFontTypeToProjectFontType(oInputFonts[0].meta().eType);
	if (oInputFonts.size() > 1)
	{
		for (size_t i = 1; i < oInputFonts.size(); ++i)
		{
			if (LibFontTypeToProjectFontType(oInputFonts[i].meta().eType) != eType)
			{
				eType = rlFont::TypefaceClassification::DontCare;
				break;
			}
		}
	}
	oFontFamilyMeta.eType = eType;

	oFontFamily.setMetadata(oFontFamilyMeta);



	// fonts/font faces
	std::map<uint64_t, size_t> oFontBySignature; // signature --> index
	for (const auto &oInputFont : oInputFonts)
	{
		const auto &oMeta = oInputFont.meta();

		const uint64_t iSignature =
			FontSignature((uint16_t)oMeta.iWeight, oMeta.bItalic, oMeta.bUnderline,
				oMeta.bStrikeOut);

		const auto it = oFontBySignature.find(iSignature);
		size_t iFont;
		if (it == oFontBySignature.end()) // add new font
		{
			rlFont::Font oFont;
			rlFont::FontMeta oFontMeta = oFont.getMetadata();
			oFontMeta.sName = std::format(L"{} {}", oMeta.iWeight,
				WeightDisplayName(oMeta.iWeight));

			if (oMeta.bItalic)
				oFontMeta.sName += L" Italic";
			if (oMeta.bStrikeOut)
				oFontMeta.sName += L" StrikeOut";
			if (oMeta.bUnderline)
				oFontMeta.sName += L" Underline";

			oFontMeta.bItalic    = oMeta.bItalic;
			oFontMeta.bStrikeout = oMeta.bStrikeOut;
			oFontMeta.bUnderline = oMeta.bUnderline;
			oFontMeta.iWeight    = oMeta.iWeight;

			oFont.setMetadata(oFontMeta);

			oFontBySignature.emplace(iSignature, oFontFamily.getFonts().size());
			oFontFamily.getFonts().push_back(std::move(oFont));
			iFont = oFontFamily.getFonts().size() - 1;
		}
		else
			iFont = it->second;

		auto &oFont = oFontFamily.getFonts()[iFont];

		rlFont::FontFace oFontFace;
		rlFont::FontFaceMeta oFontFaceMeta = oFontFace.getMetadata();
		if (!bSingleFaceName)
			oFontFaceMeta.sName = StringToWString(oMeta.sFaceName.c_str());
		oFontFaceMeta.cFallback = oMeta.cFallback;
		oFontFaceMeta.iPoints = oMeta.iPoints;
		oFontFaceMeta.sCopyright = StringToWString(oMeta.sCopyright.c_str());

		oFontFace.setMetadata(oFontFaceMeta);

		for (const auto &it : oInputFont.chars())
		{
			rlFont::Char oChar(it.second.width(), it.second.height());
			rlFont::CharMeta oCharMeta = oChar.getMetadata();
			oCharMeta.iBaseline = oMeta.iAscent;
			oChar.setMetadata(oCharMeta);

			for (unsigned iX = 0; iX < oChar.getWidth(); ++iX)
			{
				for (unsigned iY = 0; iY < oChar.getHeight(); ++iY)
				{
					oChar.setPixel(iX, iY, it.second.getPixel(iX, iY));
				}
			}

			oFontFace.getCharacters().emplace(it.first, std::move(oChar));
		}

		oFont.getFaces().push_back(std::move(oFontFace));
	}





	// try to apply settings from bottom to top

	GeneralizeSettings(oProject);





	if (!bOutputArg)
	{
		auto oOutputPath{ oInputPath };
		oOutputPath.replace_extension(L"rlFNTPROJ");
		sOutputFile = oOutputPath;
	}

	if (!oProject.saveToFile(sOutputFile.c_str()))
	{
		printf("Error creating file \"%ls\".\n", sOutputFile.c_str());
		return 1;
	}

	printf("Successfully created file \"%ls\".\n", sOutputFile.c_str());



	return 0;
}



void PrintUsage(const wchar_t *argv0)
{
	printf(
		"Usage:\n"
		"%ls InputFile [/O:OutputFile] [/CP:Codepage]\n"
		"  InputFile    The font file to be converted to a RLFNTPROJ file.\n"
		"               The following filetypes are supported:\n"
		"               * MS-DOS fonts (*.CPI)\n"
		"               * Windows raster fonts (*.FON, *.FNT)\n"
		"  /O           Specify an output filepath.  \n"
		"  OutputFile   The path for the RLFNTPROJ file to be generated.\n"
		"               If not set, the input path and filename are used.\n"
		"  /CP          Specify a fallback codepage.\n"
		"  Codepage     A Windows codepage\n"
		"\n",
		argv0
	);
}

bool GetCodepage(const wchar_t *argv, uint16_t &iDest)
{
	std::wstring s = argv;
	std::locale loc;
	for (size_t i = 0; i < s.length(); ++i)
	{
		s[i] = std::toupper(s[i], loc);
	}

	if (!s.starts_with(L"/CP:"))
		return false;
	try
	{
		const auto iCP = std::stoi(s.c_str() + 4);
		iDest = (uint16_t)iCP;
	}
	catch (...)
	{
		printf("Error: Illegal fallback codepage parameter.\n");
		exit(1);
	}

	return true;
}

std::wstring StringToWString(const char *szSrc)
{
	std::wstring sResult;
	sResult.resize(strlen(szSrc), L' ');
	for (size_t i = 0; i < sResult.length(); ++i)
	{
		sResult[i] = szSrc[i];
	}

	return sResult;
}

uint64_t FontSignature(uint16_t iWeight, bool bItalic, bool bUnderline, bool bStrikethrough)
{
	uint64_t iResult = iWeight;
	iResult <<= (6 * 8);

	if (bItalic)
		iResult |= 0x4;
	if (bUnderline)
		iResult |= 0x2;
	if (bStrikethrough)
		iResult |= 0x1;

	return iResult;
}

rlFont::TypefaceClassification LibFontTypeToProjectFontType(rl::RasterFontReader::FontType eType)
{
	using src = rl::RasterFontReader::FontType;
	using dest = rlFont::TypefaceClassification;

	switch (eType)
	{
	case src::DontCare:
		return dest::DontCare;
	case src::Roman:
		return dest::Roman;
	case src::Swiss:
		return dest::Swiss;
	case src::Modern:
		return dest::Modern;
	case src::Script:
		return dest::Script;
	case src::Decorative:
		return dest::Decorative;

	default:
		return dest::DontCare;
	}
}

const wchar_t *WeightDisplayName(unsigned iWeight)
{
	struct WeightDisplayName
	{
		unsigned iWeight;
		const wchar_t *szDisplayName;
	};

	static constexpr std::array oDisplayNames =
	{
		WeightDisplayName{ 100, L"Thin" },
		WeightDisplayName{ 200, L"Ultra Light" },
		WeightDisplayName{ 300, L"Light" },
		WeightDisplayName{ 350, L"Semi Light" },
		WeightDisplayName{ 380, L"Book" },
		WeightDisplayName{ 400, L"Regular" },
		WeightDisplayName{ 500, L"Medium" },
		WeightDisplayName{ 600, L"Semi Bold" },
		WeightDisplayName{ 700, L"Bold" },
		WeightDisplayName{ 800, L"Ultra Bold" },
		WeightDisplayName{ 900, L"Black" },
		WeightDisplayName{ 950, L"Ultra Black" }
	};
	static constexpr wchar_t szUnknown[] = L"Unknown";

	for (const auto &oDisplayName : oDisplayNames)
	{
		if (oDisplayName.iWeight == iWeight)
			return oDisplayName.szDisplayName;
	}
	return szUnknown;
}



bool internal_GetCopyright(rlFont::Font &oFont, std::wstring &sDest)
{
	sDest.clear();
	auto &oFaces = oFont.getFaces();

	for (const auto &oFace: oFaces)
	{
		const auto &sCopyright = oFace.getMetadata().sCopyright;
		if (sCopyright.empty())
			continue;

		if (sDest.empty())
			sDest = sCopyright;
		else if (sDest != sCopyright)
		{
			sDest.clear();
			return false;
		}
	}


	// copyright notes are all equal.

	// remove redundant copies in faces
	for (auto &oFace : oFaces)
	{
		rlFont::FontFaceMeta oMeta = oFace.getMetadata();
		oMeta.sCopyright.clear();
		oFace.setMetadata(oMeta);
	}

	rlFont::FontMeta oMeta = oFont.getMetadata();
	oMeta.sCopyright = sDest;
	oFont.setMetadata(oMeta);
	return true;
}

bool internal_GetFallbackChar(rlFont::Font &oFont, char32_t &cDest)
{
	cDest = 0xFFFF;
	auto &oFaces = oFont.getFaces();

	for (const auto &oFace : oFaces)
	{
		const auto cFallback = oFace.getMetadata().cFallback;
		if (cFallback == 0xFFFF)
			continue;

		if (cDest == 0xFF)
			cDest = cFallback;
		else if (cDest != cFallback)
		{
			cDest = 0xFFFF;
			return false;
		}
	}


	// fallback characters are all equal.

	// remove redundant copies in faces
	for (auto &oFace : oFaces)
	{
		rlFont::FontFaceMeta oMeta = oFace.getMetadata();
		oMeta.cFallback = 0xFFFF;
		oFace.setMetadata(oMeta);
	}

	rlFont::FontMeta oMeta = oFont.getMetadata();
	oMeta.cFallback = cDest;
	oFont.setMetadata(oMeta);
	return true;
}

void GeneralizeSettings(rlFont::Project &oProject)
{
	auto &oFontFamily = oProject.getFontFamily();

	rlFont::FontFamilyMeta oMeta = oFontFamily.getMetdata();

	bool bSingleValue = true;



	// copyright

	std::wstring sCopyright;
	std::wstring sTMP;
	for (auto &oFont : oFontFamily.getFonts())
	{
		bSingleValue = internal_GetCopyright(oFont, sTMP);
		if (!bSingleValue)
			break;
		if (sCopyright.empty())
			sCopyright = sTMP;
		else if (!sTMP.empty() &&  sCopyright != sTMP)
		{
			bSingleValue = false;
			break;
		}
	}
	if (bSingleValue)
	{
		oMeta.sCopyright = sCopyright;

		for (auto &oFont : oFontFamily.getFonts())
		{
			rlFont::FontMeta oFontMeta = oFont.getMetadata();
			oFontMeta.sCopyright.clear();
			oFont.setMetadata(oFontMeta);
		}
	}



	// fallback character
	char32_t cFallback = 0xFFFF;
	char32_t cTMP;
	for (auto &oFont : oFontFamily.getFonts())
	{
		bSingleValue = internal_GetFallbackChar(oFont, cTMP);
		if (!bSingleValue)
			break;
		if (cFallback == 0xFFFF)
			cFallback = cTMP;
		else if (cTMP != 0xFFFF && cFallback != cTMP)
		{
			bSingleValue = false;
			break;
		}
	}
	if (bSingleValue)
	{
		oMeta.cFallback = cFallback;

		for (auto &oFont : oFontFamily.getFonts())
		{
			rlFont::FontMeta oFontMeta = oFont.getMetadata();
			oFontMeta.cFallback = 0xFFFF;
			oFont.setMetadata(oFontMeta);
		}
	}


	oFontFamily.setMetadata(oMeta);
}
