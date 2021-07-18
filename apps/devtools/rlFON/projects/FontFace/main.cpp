#include <rl/console.hpp>
#include "..\..\comps\consoleui.hpp"
#include "..\..\comps\parser.hpp"
#include "..\..\comps\rlfnt.hpp"

#include <conio.h> // _getch()
#include <cwctype> // std::iswcntrl()
#include <memory> // smart pointers
#include <winerror.h> // ERROR_[...]





//==================================================================================================
// SYNTAX
/*

FontFace validate <File>
	Summary
		Check if a rlFNT file is valid

	Params
		File			The rlFNT file to validate



FontFace create <OutputPath> <InputPath> <CodePage>
	Summary
		Create a rlFNT file from a Microsoft FNT file

	Params
		OutputPath		output rlFNT file
		InputPath		input FNT file
		CodePage		Windows CodePage of the FNT



FontFace info <File>
	Summary
		Get a short overview about a rlFNT file

	Params
		File			the rlFNT file to analyze



FontFace list <File>
	Summary
		List all unicode characters present in the font

	Params
		File			the rlFNT file to analyze



FontFace mergeinto <DestFile> <SrcFile> <CodePage>
	Summary
		Merge a Microsoft FNT file into a rlFNT file

	Params
		OutputPath		destination rlFNT file
		SrcFile			source FNT file
		CodePage		Windows CodePage of the source FNT file



FontFace preview <File> <CharHex>
	Summary
		Preview a single char from a rlFNT file

	Params
		File			the rlFNT the character should be extracted from
		CharHex			hexadecimal value of the unicode character to preview



FontFace setstring <DestFile> <StringName> "<NewStringVal>"
	Summary
		Change one of a rlFNT's strings

	Params
		DestFile		destination rlFNT file
		StringName		one of the following values:
							Copyright		The face's copyright string
							FontFamily		The face's font family name
							FontFace		The face's name
											(treated as a suffix for FontFamily, if not empty)
		NewStringVal	The new value for the string identified by StringName
						(quotes are escaped by being doubled; only ASCII characters are accepted)

*/



/// <summary>
/// Show the correct syntax for the application
/// </summary>
void ShowSyntax();

/// <summary>
/// Write a rlFNT validation error to the console
/// </summary>
void WriteFontFaceError(rl::BitmapFont::Face::FileStatus status);

/// <summary>
/// Decode a CodePage character to it's raw unicode value
/// </summary>
bool Decode(uint8_t ch, uint16_t codepage, uint32_t& result);

/// <summary>
/// Check if a path is valid for writing
/// </summary>
/// <returns>
/// If the path is valid for writing: <c>ERROR_SUCCESS</c> (0)<para/>
/// Else: A return value for main() to return
/// </returns>
int CheckOutputPath(const wchar_t* szPath);

/// <summary>
/// Draw a "pixel" into the console<para/>
/// Draws either two full blocks (U+2588) or two spaces (U+20)
/// </summary>
void DrawPixel(bool bVisible);

/// <summary>
/// Get a confirmation from the user, with "y"/"Y"/"n"/"N"
/// </summary>
/// <param name="bPrintValidInput">= should "Y"/"N" be printed to the console?</param>
/// <returns>Did the user input "y"/"Y" (as opposed to "n"/"N")?</returns>
bool UserConfirmation(bool bPrintValidInput);

/// <summary>
/// Print a single bitmap character to the console (with an outline)
/// </summary>
/// <param name="ch"></param>
void PrintChar(const rl::BitmapFont::Char& ch);

/// <summary>
/// Check if a path is a valid rlFNT file
/// </summary>
/// <returns>Is the file valid?</returns>
bool CheckFile(const wchar_t* szPath);

/// <summary>
/// [mergeinto] Let the user choose between the current and the new string value
/// (if it's currently not empty)
/// </summary>
/// <param name="szStringNameSingle">= the name of the string</param>
/// <param name="szStringNameMultiple">= the name of multiple of the strings</param>
/// <param name="szCurrent">= the string value currently in the file</param>
/// <param name="szNew">= the potential replacement</param>
/// <param name="res">= the result chosen by either the program or the user</param>
void UserMergeStringChoice(const char* szStringNameSingle, const char* szStringNameMultiple,
	const char* szCurrent, const char* szNew, std::string& res);

/// <summary>
/// Try saving a font face, ask user for retry if not successful
/// </summary>
void TrySaving(const rl::BitmapFont::Face& face, const wchar_t* szPath);



//==================================================================================================
// ENTRY POINT
int wmain(int argc, wchar_t* argv[])
{
	rl::Console::PushColor(BG_BLACK | FG_GRAY); // default colors
	const char szAppName[] = "FontFace";
	printf("\n");

	// check argument count
	if (argc < 3)
	{
		rl::WriteHelpHint(szAppName);
		printf("\n");
		return ERROR_BAD_ARGUMENTS;
	}

	// define bonus characters (not defined in the codepages, but still always the same if defined)
	std::map<WORD, std::map<uint8_t, uint32_t>> oBonusChars;

	std::map<uint8_t, uint32_t> oChars;
	oBonusChars.emplace(932, oChars);
	auto& oCPBonus = oBonusChars[932];
	oCPBonus.emplace(1, 0x2554);
	oCPBonus.emplace(2, 0x2557);
	oCPBonus.emplace(3, 0x255A);
	oCPBonus.emplace(4, 0x255D);
	oCPBonus.emplace(5, 0x2551);
	oCPBonus.emplace(6, 0x2550);
	oCPBonus.emplace(7, 0x2193);
	oCPBonus.emplace(9, 0x9675);
	oCPBonus.emplace(11, 0xFFFD); // replacement character
	oCPBonus.emplace(14, 0x2022);
	oCPBonus.emplace(15, 0x263C);
	oCPBonus.emplace(16, 0x256C);
	oCPBonus.emplace(18, 0x2195);
	oCPBonus.emplace(20, 0x2592);
	oCPBonus.emplace(21, 0x2569);
	oCPBonus.emplace(22, 0x2566);
	oCPBonus.emplace(23, 0x2563);
	oCPBonus.emplace(25, 0x2560);
	oCPBonus.emplace(26, 0x2592);
	oCPBonus.emplace(27, 0x21B5);
	oCPBonus.emplace(28, 0x2191);
	oCPBonus.emplace(29, 0x2502);
	oCPBonus.emplace(30, 0x2192);
	oCPBonus.emplace(31, 0x2190);



	// prepare the 1st argument (command)
	size_t len = wcslen(argv[1]) + 1;
	auto up_szCommand = std::make_unique<wchar_t[]>(len);
	wchar_t* szCommand = up_szCommand.get();
	wcscpy_s(szCommand, len, argv[1]);
	_wcslwr_s(szCommand, len); // convert command to all lowercase


	rl::BitmapFont::Face rlFNT;

	const uint32_t iPrivateUseAreaBegin = 0xE000; // first valid private use area value
	const uint32_t iPrivateUseAreaEnd = 0xF8FF; // last valid private use area value



	// CLEANUP *************************************************************************************
	if (wcscmp(szCommand, L"cleanup") == 0)
	{
		if (argc != 3)
		{
			rl::WriteHelpHint(szAppName);
			printf("\n");
			return ERROR_BAD_ARGUMENTS;
		}

		const wchar_t* szPath = argv[2];

		if (!CheckFile(szPath))
			return ERROR_FILE_CORRUPT;

		rlFNT.loadFromFile(szPath);





		// delete empty chars (ignore ' ' and NBSP (0xA0)
		{
			std::vector<uint32_t> oEmptyChars;

			for (auto& oCh : rlFNT)
			{
				if (oCh.first == ' ' || oCh.first == 0xA0)
					continue;

				if (oCh.second.isEmpty())
					oEmptyChars.push_back(oCh.first);
			}

			if (oEmptyChars.size() > 0)
			{
				for (uint32_t i : oEmptyChars)
					rlFNT.remove(i);

				printf("Deleted %zu empty chars\n", oEmptyChars.size());
			}
		}





		// A bitmap font character and the codepoints of all of it's duplicates
		struct DupChar
		{
			rl::BitmapFont::Char ch; // character data
			std::vector<uint32_t> cps; // codepoints with this character
		};

		std::vector<DupChar> oDups; // list of duplicates





		// delete all duplicates within the private use area
		{
			std::vector<DupChar> oDupsPU;

			// go through all of the private use area to detect duplicates
			for (const auto& o : rlFNT)
			{
				// skip characters before the private use area
				if (o.first < iPrivateUseAreaBegin)
					continue;

				// if beyond private use area, exit the duplicate search
				if (o.first > iPrivateUseAreaEnd)
					break;



				// search if there is a duplicate group for this character yet
				bool bGroupFound = false;
				for (auto& grp : oDupsPU)
				{
					if (o.second == grp.ch)
					{
						grp.cps.push_back(o.first);
						bGroupFound = true;
					}
				}

				// if there is no group for this character yet, create it
				if (!bGroupFound)
				{
					DupChar dch = { o.second, std::vector<uint32_t>{o.first} };
					oDupsPU.push_back(dch);
				}
			}

			// remove all non-duplicate characters
			for (auto it = oDupsPU.cbegin(); it != oDupsPU.cend(); /*no increment*/)
			{
				// add first, not-to-be-removed character, to the general duplicate vector
				oDups.push_back(DupChar{ it->ch, std::vector<uint32_t>{it->cps[0]} });

				if (it->cps.size() == 1)
					it = oDupsPU.erase(it);
				else
					++it;
			}


			if (oDupsPU.size() > 0)
			{
				// get the count of redundant characters
				uint64_t iDupCharCount = 0;
				for (auto& o : oDupsPU)
					iDupCharCount += o.cps.size() - 1;

				// keep 1st iteration of every redundant char
				for (auto& o : oDupsPU)
					for (size_t i = 1; i < o.cps.size(); i++)
						rlFNT.remove(o.cps[i]);

				printf("Deleted %llu redundancies total of %zu characters in the private use area"
					"\n", iDupCharCount, oDupsPU.size());
			}
		}





		// find and delete all characters in the private use area that are duplicates of characters
		// outside the private use area
		{
			// oDups is at this point filled with all the characters remaining in the private use
			// area (duplicates removed)

			for (auto& o : rlFNT)
			{
				// skip private use area
				if (o.first >= iPrivateUseAreaBegin && o.first <= iPrivateUseAreaEnd)
					continue;


				// if one of the private use area characters, add codepoint
				for (auto& grp : oDups)
					if (o.second == grp.ch)
						grp.cps.push_back(o.first);
			}

			uint32_t iDupCount = 0;
			for (auto& o : oDups)
			{
				if (o.cps.size() > 1)
				{
					iDupCount++;
					rlFNT.remove(o.cps[0]);
				}
			}

			printf("Deleted %u characters from the private use area that were duplicates of "
				"characters outside the private use area\n\n", iDupCount);

			oDups.clear();
		}






		// compress the private use area by moving all characters to the start
		{
			std::map<uint32_t, rl::BitmapFont::Char> oPUChars;
			std::vector<uint32_t> oPUCharsPrev; // chars to delete
			uint32_t iCP = iPrivateUseAreaBegin;

			for (auto& o : rlFNT)
			{
				// skip characters before the private use area
				if (o.first < iPrivateUseAreaBegin)
					continue;

				// if beyond private use area, exit
				if (o.first > iPrivateUseAreaEnd)
					break;


				oPUChars.emplace(iCP, o.second);
				oPUCharsPrev.push_back(o.first);

				iCP++;
			}

			// delete the characters at their current positions
			for (uint32_t i : oPUCharsPrev)
				rlFNT.remove(i);

			// add the characters back in with the new ID
			for (auto& o : oPUChars)
				rlFNT.set(o.first, o.second);


			printf("Moved the private use area chars to the start of the private use area\n\n");
		}





		TrySaving(rlFNT, szPath);

		printf("\n\n");





		// search for identical characters
		std::vector<std::vector<uint32_t>> oIdenticalChars;

		bool bFound;
		for (auto& oCh : rlFNT)
		{
			if (oCh.second.isEmpty())
				continue; // empty chars were already checked before

			bFound = false;
			for (auto& oGroup : oIdenticalChars)
			{
				if (rlFNT.getChar(oGroup[0]) == oCh.second)
				{
					bFound = true;
					oGroup.push_back(oCh.first);
					break;
				}
			}
			if (!bFound)
				oIdenticalChars.push_back({ oCh.first });
		}

		// ask user if duplicated characters are dummies
		for (auto& oGroup : oIdenticalChars)
		{
			if (oGroup.size() > 1)
			{
				PrintChar(rlFNT.getChar(oGroup[0]));
				printf("\nThis character appeared in the rlFNT %zu times.\n"
					"\nIs it a dummy character (like a solid block) that should be deleted? [y/n] ",
					oGroup.size());
				if (UserConfirmation(true))
				{
					for (uint32_t iCP : oGroup)
						rlFNT.remove(iCP);

					TrySaving(rlFNT, szPath);

					printf("\n\n%zu characters deleted.\n\n", oGroup.size());
					break; // only delete one duplicate character
				}
			}
		}

		printf("Cleanup done.\n\n");
	}



	// CREATE **************************************************************************************
	else if (wcscmp(szCommand, L"create") == 0)
	{
		if (argc != 5)
		{
			rl::WriteHelpHint(szAppName);
			printf("\n");
			return ERROR_BAD_ARGUMENTS;
		}


		const wchar_t* szOutputPath = argv[2];
		const wchar_t* szInputPath = argv[3];
		const wchar_t* szCodePage = argv[4];
		DWORD dwLastError = 0;

		int iWrite = CheckOutputPath(szOutputPath);
		if (iWrite != 0)
			return iWrite;

		const size_t lenCP = wcslen(szCodePage);
		if (lenCP == 0)
		{
			rl::WriteHelpHint(szAppName); // codepage argument was empty
			printf("\n");
			return ERROR_BAD_ARGUMENTS;
		}
		wchar_t* pCpEnd = nullptr;
		const uint32_t iCodePage = wcstoul(szCodePage, &pCpEnd, 10);
		if (pCpEnd - szCodePage != lenCP)
		{
			rl::WriteHelpHint(szAppName); // codepage argument was not a valid number
			printf("\n");
			return ERROR_BAD_ARGUMENTS;
		}

		if (!IsValidCodePage(iCodePage))
		{
			rl::WriteError("%d is not a valid Windows CodePage", iCodePage);
			return ERROR_BAD_ARGUMENTS;
		}

		rl::MicrosoftRasterFont src;
		if (!src.loadFromFile(szInputPath))
		{
			rl::WriteFONError(src.getError());
			return ERROR_FILE_CORRUPT;
		}

		rl::FONTHDR hdr = {};
		src.getHeader(hdr);

		// check for fixed width
		uint32_t iFixedWidth = hdr.dfPixWidth;
		if (iFixedWidth == 0)
		{
			if ((hdr.dfPitchAndFamily & 1) == 0) // fixed pitch given?
			{
				iFixedWidth = src.begin()->second.getWidth();
			}
			else // check if all characters have the same width
			{
				uint32_t iWidth = 0;
				for (auto& o : src)
				{
					if (iWidth == 0)
						iWidth = o.second.getWidth();
					else if (o.second.getWidth() != iWidth)
					{
						iWidth = 0;
						break;
					}
				}

				// if de-facto fixed-width, ask user if rlFNT should be created fixed-width
				if (iWidth > 0)
				{
					printf("Even though the input font is not declared fixed-width, all of it's "
						"characters are %d pixels wide.\n"
						"Should a fixed-width rlFNT be created from it? [y/n] ", iWidth);

					if (UserConfirmation(true))
						iFixedWidth = iWidth;
				}
			}
		}

		uint8_t iFlags = 0;
		if (hdr.dfItalic)
			iFlags |= RL_FNT_FLAG_ITALIC;
		if (hdr.dfPitchAndFamily & FF_ROMAN)
			iFlags |= RL_FNT_FLAG_SERIFS;

		// guarantee for copyright string to be null-terminated
		char szCopyright[61];
		szCopyright[60] = 0;
		memcpy_s(szCopyright, 60, hdr.dfCopyright, 60);

		// trim spaces from end of copyright string
		for (uint8_t i = 61; i > 0; i--)
		{
			if (szCopyright[i - 1] == ' ' || szCopyright[i - 1] == 0)
				szCopyright[i - 1] = 0;
			else
				break;
		}

		// create the rlFNT in memory
		// in praxis, the "face name" of a FNT file is really the family name most of the time
		rlFNT.create(iFixedWidth, hdr.dfPixHeight, 1, hdr.dfPoints, hdr.dfWeight / 10, iFlags,
			src.faceName(), nullptr, szCopyright);

		uint32_t iCodepoint_Custom = iPrivateUseAreaBegin;
		uint16_t iCodePage_File = 0;
		if (rl::CharSetToCodePage(hdr.dfCharSet, iCodePage_File) && iCodePage != iCodePage_File)
			rl::WriteWarning("CodePage difference: File says CodePage %d, user says CodePage %d",
				iCodePage_File, iCodePage);

		uint32_t iCodePoint = 0;
		uint32_t iFallback = 0;
		for (auto& o : src) // go through all characters
		{
			const uint8_t iCharSetID = o.first;

			// push undefined characters and control characters into private use area
			if (!Decode(iCharSetID, iCodePage, iCodePoint) || std::iswcntrl(iCodePoint))
			{
				// search for unknown characters in the translation map for bonus characters
				bool bFound = false;
				auto it = oBonusChars.find(iCodePage);
				if (it != oBonusChars.end())
				{
					auto it2 = it->second.find(iCharSetID);
					if (it2 != it->second.end())
					{
						bFound = true;
						iCodePoint = it->second.at(iCharSetID);
					}
				}

				if (!bFound)
				{
					iCodePoint = iCodepoint_Custom;
					iCodepoint_Custom++;
				}
			}

			// if this is the fallback character, save the unicode codepoint
			if (iCharSetID == hdr.dfDefaultChar)
				iFallback = iCodePoint;

			auto& ch = rlFNT.add(iCodePoint, o.second.getWidth());

			// copy graphics data
			for (uint32_t iY = 0; iY < ch.getHeight(); iY++)
			{
				for (uint32_t iX = 0; iX < ch.getWidth(); iX++)
				{
					ch.setPixel(iX, iY, o.second.getPixel((uint16_t)iX, (uint16_t)iY));
				}
			}
		}
		rlFNT.setFallback(iFallback);

		if (!rlFNT.saveToFile(szOutputPath))
		{
			rl::WriteError("Couldn't save to file \"%ls\"", szOutputPath);
			return -1;
		}

		printf("Successfully created rlFNT file \"%ls\"\n", szOutputPath);
	}



	// INFO ****************************************************************************************
	else if (wcscmp(szCommand, L"info") == 0)
	{
		if (argc != 3)
		{
			rl::WriteHelpHint(szAppName);
			printf("\n");
			return ERROR_BAD_ARGUMENTS;
		}

		const wchar_t* szPath = argv[2];

		if (!CheckFile(szPath))
			return ERROR_FILE_CORRUPT;


		rlFNT.loadFromFile(szPath);

		const auto& sFamilyName = rlFNT.getFamilyName();
		const auto& sFaceName = rlFNT.getFaceName();
		const auto& sCopyright = rlFNT.getCopyright();

		const char szUndef[] = "<undefined>";
		printf("Font Family:  %s\n", sFamilyName.empty() ? szUndef : sFamilyName.c_str());
		printf("Font Face:    %s\n", sFaceName.empty() ? szUndef : sFaceName.c_str());
		printf("Copyright:    %s\n\n", sCopyright.empty() ? szUndef : sCopyright.c_str());
		uint8_t iVersion[4];
		rlFNT.getVersion(iVersion);
		printf("Version:      %d.%d.%d.%d\n", iVersion[0], iVersion[1], iVersion[2], iVersion[3]);
		printf("Height:       %d Px\n", rlFNT.getHeight());
		const uint16_t iFixedWidth = rlFNT.getFixedWidth();
		if (iFixedWidth > 0)
			printf("Fixed width:  %d Px\n", rlFNT.getFixedWidth());
		else
			printf("Variable width\n");
		printf("Weight:       %u (%s)\n", rlFNT.getWeight(),
			rl::BitmapFontWeightStrings::GetString(rlFNT.getWeight()));
		const uint8_t iFlags = rlFNT.getFlags();
		printf("Flags:        ");
		if (iFlags == 0)
			printf("0");
		else
		{
#define PRINTSTR(s)						\
				if (iFlags & s)			\
				{						\
					if (bFlag)			\
						printf(" | ");	\
					printf(#s);			\
					bFlag = true;		\
				}


			bool bFlag = false; // has a flag already been written? (for "|")

			PRINTSTR(RL_FNT_FLAG_ITALIC);
			PRINTSTR(RL_FNT_FLAG_SERIFS);

#undef PRINTSTR
		}
		printf("\n");
		printf("Points:       %d Pt\n", rlFNT.getPoints());

		printf("%zu Characters\n", rlFNT.charCount());

		printf("\nFallback character: U+%04X\n", rlFNT.getFallback());
	}



	// LIST ****************************************************************************************
	else if (wcscmp(szCommand, L"list") == 0)
	{
		if (argc != 3)
		{
			rl::WriteHelpHint(szAppName);
			printf("\n");
			return ERROR_BAD_ARGUMENTS;
		}

		const wchar_t* szPath = argv[2];

		if (!CheckFile(szPath))
			return ERROR_FILE_CORRUPT;


		rlFNT.loadFromFile(szPath);


		printf("%zu unicode codepoints in rlFNT file \"%ls\":\n\n", rlFNT.charCount(),szPath);

		for (auto& o : rlFNT)
			printf("U+%04X\n", o.first);

		printf("\n\n");
	}



	// MERGEINTO ***********************************************************************************
	else if (wcscmp(szCommand, L"mergeinto") == 0)
	{
		if (argc != 5)
		{
			rl::WriteHelpHint(szAppName);
			printf("\n");
			return ERROR_BAD_ARGUMENTS;
		}

		const wchar_t* szDestFile = argv[2];
		const wchar_t* szSrcFile = argv[3];
		const wchar_t* szCodePage = argv[4];

		if (!CheckFile(szDestFile))
			return ERROR_FILE_CORRUPT;

		rlFNT.loadFromFile(szDestFile);

		if (rlFNT.getBitsPerPixel() != 1)
		{
			rl::WriteError("Not mergeable: Destination file is not 1BPP");
			return -1;
		}


		const size_t lenCP = wcslen(szCodePage);
		if (lenCP == 0)
		{
			rl::WriteHelpHint(szAppName); // codepage argument was empty
			printf("\n");
			return ERROR_BAD_ARGUMENTS;
		}
		wchar_t* pCpEnd = nullptr;
		const uint32_t iCodePage = wcstoul(szCodePage, &pCpEnd, 10);
		if (pCpEnd - szCodePage != lenCP)
		{
			rl::WriteHelpHint(szAppName); // codepage argument was not a valid number
			printf("\n");
			return ERROR_BAD_ARGUMENTS;
		}

		if (!IsValidCodePage(iCodePage))
		{
			rl::WriteError("%d is not a valid Windows CodePage", iCodePage);
			return ERROR_BAD_ARGUMENTS;
		}

		rl::MicrosoftRasterFont src;
		if (!src.loadFromFile(szSrcFile))
		{
			rl::WriteFONError(src.getError());
			return ERROR_FILE_CORRUPT;
		}

		rl::FONTHDR hdr = {};
		src.getHeader(hdr);

		if (rlFNT.getHeight() != hdr.dfPixHeight)
		{
			rl::WriteError("Not mergeable: Different font heights (destination: %u, to merge: %u)",
				rlFNT.getHeight(), hdr.dfPixHeight);
			return -1;
		}


		// fixed width
		const uint32_t iFixedWidthDest = rlFNT.getFixedWidth();

		uint32_t iFixedWidthSrc = hdr.dfPixWidth;
		if ((hdr.dfPitchAndFamily & 1) == 0) // fixed pitch given?
		{
			iFixedWidthSrc = src.begin()->second.getWidth();
		}
		else // check if all characters have the same width
		{
			uint32_t iWidth = 0;
			for (auto& o : src)
			{
				if (iWidth == 0)
					iWidth = o.second.getWidth();
				else if (o.second.getWidth() != iWidth)
				{
					iWidth = 0;
					break;
				}
			}

			// if de-facto fixed-width, ask user if rlFNT should be created fixed-width
			if (iWidth > 0 && iWidth == iFixedWidthDest)
			{
				printf("Even though the font to merge is not declared fixed-width, all of it's "
					"characters are %d pixels wide, which makes it a match for the current font.\n"
					"Should the fonts be merged? [y/n] ", iWidth);

				if (UserConfirmation(true))
					iFixedWidthSrc = iWidth;
				else
				{
					printf("\nMerging cancelled.\n");
					return ERROR_CANCELLED;
				}
			}
		}

		// different fixed width parameter
		if (iFixedWidthSrc != iFixedWidthDest)
		{
			// destination fixed width is set --> offer converting to non-fixed-width
			if (iFixedWidthDest != 0)
			{
				if (iFixedWidthSrc != 0)
					printf("The fonts have different fixed widths set (%u and %u).",
						iFixedWidthDest, iFixedWidthSrc);
				else
					printf("The font is currently fixed-width, but the font to merge is "
						"variable-width.");


				printf("\nShould the font be recreated as a non-fixed-width font? [y/n] ");
				if (!UserConfirmation(true))
				{
					printf("\nMerging cancelled.\n");
					return ERROR_CANCELLED;
				}


				// user has allowed recreation as variable-width font --> do that

				rl::BitmapFont::Face rlFNT2;
				rlFNT2.create(0, rlFNT.getHeight(), rlFNT.getBitsPerPixel(),
					rlFNT.getPoints(), rlFNT.getWeight(), rlFNT.getFlags(),
					rlFNT.getFamilyName().c_str(), rlFNT.getFaceName().c_str(),
					rlFNT.getCopyright().c_str());

				for (auto& o : rlFNT)
					rlFNT.set(o.first, o.second);

				rlFNT = rlFNT2;
			}
		}



		// at this point, the data of the input file can be merged into the destination file without
		// worries

		uint16_t iCodePage_File = 0;
		if (rl::CharSetToCodePage(hdr.dfCharSet, iCodePage_File) && iCodePage != iCodePage_File)
			rl::WriteWarning("CodePage difference: File says CodePage %d, user says CodePage %d",
				iCodePage_File, iCodePage);

		uint32_t iCodePoint;
		uint32_t iCodePoint_Custom = iPrivateUseAreaBegin;
		uint32_t iFallback = 0;
		for (auto& o : src)
		{
			const uint8_t iCharSetID = o.first;

			// push undefined characters and control characters into private use area
			if (!Decode(iCharSetID, iCodePage, iCodePoint) || std::iswcntrl(iCodePoint))
			{
				// search for unknown characters in the translation map for bonus characters
				bool bFound = false;
				auto it = oBonusChars.find(iCodePage);
				if (it != oBonusChars.end())
				{
					auto it2 = it->second.find(iCharSetID);
					if (it2 != it->second.end())
					{
						bFound = true;
						iCodePoint = it->second.at(iCharSetID);
					}
				}

				if (!bFound)
				{
					// skip custom chars already defined
					while (rlFNT.containsChar(iCodePoint_Custom))
						iCodePoint_Custom++;

					iCodePoint = iCodePoint_Custom;
					iCodePoint_Custom++;
				}
			}

			// if this is the fallback character, save the unicode codepoint
			if (iCharSetID == hdr.dfDefaultChar)
				iFallback = iCodePoint;

			rl::BitmapFont::Char chNew;

			// destination already contains this character
			if (rlFNT.containsChar(iCodePoint))
			{
				chNew.create(1, o.second.getWidth(), o.second.getHeight());

				// copy graphics data
				for (uint32_t iY = 0; iY < chNew.getHeight(); iY++)
				{
					for (uint32_t iX = 0; iX < chNew.getWidth(); iX++)
					{
						chNew.setPixel(iX, iY, o.second.getPixel((uint16_t)iX, (uint16_t)iY));
					}
				}

				const auto& chOld = rlFNT.getChar(iCodePoint);

				// characters are identical --> skip
				if (chOld == chNew)
					continue;


				// characters are different --> let the user decide
				PrintChar(chOld);
				printf("^^ current data ^^\n\n");
				PrintChar(chNew);
				printf("^^ new data ^^\n\n");

				printf("These are the two options for the character U+%04X\n"
					"Should the char be replaced? [y/n] ", iCodePoint);
				if (UserConfirmation(true))
					rlFNT.set(iCodePoint, chNew);
			}

			// the character is not yet present in the font --> copy the data
			else
			{
				auto& ch = rlFNT.add(iCodePoint, o.second.getWidth());

				// copy graphics data
				for (uint32_t iY = 0; iY < ch.getHeight(); iY++)
				{
					for (uint32_t iX = 0; iX < ch.getWidth(); iX++)
					{
						ch.setPixel(iX, iY, o.second.getPixel((uint16_t)iX, (uint16_t)iY));
					}
				}
			}
		}

		const uint32_t iCurrentFallback = rlFNT.getFallback();
		if (iFallback != iCurrentFallback)
		{
			// let the user decide for a fallback character

			PrintChar(rlFNT.getChar(iCurrentFallback));
			printf("^^ current fallback ^^\n\n");
			PrintChar(rlFNT.getChar(iFallback));
			printf("^^ new fallback ^^\n\n");

			printf("These are the two options for the fallback character.\n"
				"Should the fallback character be updated? [y/n] ");
			if (UserConfirmation(true))
				rlFNT.setFallback(iFallback);
		}



		std::string sFamName = rlFNT.getFamilyName();
		UserMergeStringChoice("font family name", "font family names", sFamName.c_str(),
			src.faceName(), sFamName);
		rlFNT.setFamilyName(sFamName.c_str());

		// there is no face name in a Microsoft FNT file - the "face name" is the family name.

		std::string sCopyright = rlFNT.getCopyright();

		// insure the copyright string is zero-terminated
		char szCopyright[61];
		szCopyright[60] = 0;
		memcpy_s(szCopyright, 61, hdr.dfCopyright, 60);
		// remove spaces at the end
		for (int8_t i = 59; i >= 0; i--)
		{
			char& c = szCopyright[i];
			if (c == ' ')
				c = 0;
			else if (c != 0)
				break;
		}

		UserMergeStringChoice("copyright text", "copyright texts", sCopyright.c_str(), szCopyright,
			sCopyright);

		rlFNT.setCopyright(sCopyright.c_str());



		if (!rlFNT.saveToFile(szDestFile))
		{
			rl::WriteError("Couldn't save to file \"%ls\"", szDestFile);
			return -1;
		}

		printf("Successfully merged the two files into \"%ls\"\n", szDestFile);
	}



	// PREVIEW *************************************************************************************
	else if (wcscmp(szCommand, L"preview") == 0)
	{
		if (argc != 4)
		{
			rl::WriteHelpHint(szAppName);
			printf("\n");
			return ERROR_BAD_ARGUMENTS;
		}

		const wchar_t* szFile = argv[2];
		const wchar_t* szHex = argv[3];

		if (!CheckFile(szFile))
			return ERROR_FILE_CORRUPT;

		// check the hex code
		const size_t lenHex = wcslen(szHex);
		for (size_t i = 0; i < lenHex; i++)
		{
			wchar_t c = szHex[i];

			// if char, to uppercase
			if (c > 'a' && c < 'z')
				c = c - 'a' + 'A';

			if (szHex[i] < '0' || szHex[i] > 'Z' || (szHex[i] > '9' && szHex[i] < 'A'))
			{
				rl::WriteError("\"%ls\" is no valid hex string", szHex);
				return ERROR_BAD_ARGUMENTS;
			}
		}

		if (lenHex > 2 * sizeof(uint32_t))
		{
			rl::WriteError("Hex string was too long - maximum is %u hex chars",
				(unsigned)sizeof(uint32_t) * 2);
			return ERROR_BAD_ARGUMENTS;
		}

		uint32_t iCodepoint = wcstoul(szHex, nullptr, 16);

		rlFNT.loadFromFile(szFile);
		if (!rlFNT.containsChar(iCodepoint))
		{
			rl::WriteError("Character U+%04X is not defined in this font face\n\n", iCodepoint);
			return -1;
		}

		const auto& ch = rlFNT.getChar(iCodepoint);

		PrintChar(ch);
		printf("U+%04X\n", iCodepoint);
		printf("Width = %u     Height = %u\n", ch.getWidth(), ch.getHeight());
	}



	// SETSTRING ***********************************************************************************
	else if (wcscmp(szCommand, L"setstring") == 0)
	{
		if (argc != 5)
		{
			rl::WriteHelpHint(szAppName);
			printf("\n");
			return ERROR_BAD_ARGUMENTS;
		}

		const wchar_t* szFile = argv[2];
		const wchar_t* szStringName = argv[3];
		const wchar_t* szStringVal = argv[4];


		// prepare string name for comparison (copy + convert to all lowercase)
		size_t lenStrName = wcslen(szStringName) + 1;
		auto up_szStrName = std::make_unique<wchar_t[]>(lenStrName);
		wchar_t* szStrName = up_szStrName.get();
		wcscpy_s(szStrName, lenStrName, szStringName);
		_wcslwr_s(szStrName, lenStrName); // convert string name to all lowercase

		const size_t lenStrVal = wcslen(szStringVal) + 1;
		auto up_szStrValA = std::make_unique<char[]>(lenStrVal);
		char* szStrValA = up_szStrValA.get();

		szStrValA[lenStrVal - 1] = 0;
		for (size_t i = 0; i < lenStrVal - 1; i++)
		{
			wchar_t c = szStringVal[i];
			if (c > 0x7F)
			{
				rl::WriteError("Invalid string value: Only ASCII characters are allowed!");
				return ERROR_BAD_ARGUMENTS;
			}

			szStrValA[i] = (char)c;
		}



		if (!CheckFile(szFile))
			return ERROR_FILE_CORRUPT;

		rlFNT.loadFromFile(szFile);

		if (wcscmp(szStrName, L"fontfamily") == 0)
			rlFNT.setFamilyName(szStrValA);
		else if (wcscmp(szStrName, L"fontface") == 0)
			rlFNT.setFaceName(szStrValA);
		else if (wcscmp(szStrName, L"copyright") == 0)
			rlFNT.setCopyright(szStrValA);
		else
		{
			rl::WriteError("\"%ls\" is not the name of a known string.", szStringName);
			return ERROR_BAD_ARGUMENTS;
		}

		if (!rlFNT.saveToFile(szFile))
		{
			rl::WriteError("Couldn't save to file \"%ls\"", szFile);
			return -1;
		}


		printf("Successfully updated string \"%ls\" to \"%s\"\n", szStrName, szStrValA);
	}



	// VALIDATE ************************************************************************************
	else if (wcscmp(szCommand, L"validate") == 0)
	{
		if (argc != 3)
		{
			rl::WriteHelpHint(szAppName);
			printf("\n");
			return ERROR_BAD_ARGUMENTS;
		}

		const wchar_t* szPath = argv[2];

		if (!CheckFile(szPath))
			return ERROR_FILE_CORRUPT;

		printf("File \"%ls\" is a valid rlFNT file\n\n", szPath);
	}



	// UNKNOWN COMMAND *****************************************************************************
	else
	{
		rl::WriteHelpHint(szAppName);
		printf("\n");
		return ERROR_BAD_ARGUMENTS;
	}





	printf("\n\n");

	return ERROR_SUCCESS;
}


void ShowSyntax()
{
	printf("\n"
		"usage options:\n"
		"\n"
		"FontFace cleanup <File>\n"
		"    Search for ways to optimize the rlFNT file <File>\n"
		"\n"
		"FontFace create <OutputPath> <InputPath> <InputCodePage>\n"
		"    Create a new rlFNT file at <OutputPath> from Microsoft FNT file <InputPath>,\n"
		"    using codepage <InputCodePage>\n"
		"\n"
		"FontFace info <File>\n"
		"    Get basic information about a rlFNT file\n"
		"\n"
		"FontFace list <File>\n"
		"    List all unicode codepoints in the rlFNT file <File>\n"
		"\n"
		"FontFace mergeinto <DestFile> <SrcFile> <SrcCodePage>\n"
		"    Merge the data from the Microsoft FNT file <SrcFile> into the rlFNT file <DestFile>,\n"
		"    using CodePage <SrcCodePage>\n"
		"\n"
		"FontFace preview <File> <CharHex>\n"
		"    Print a preview of the character with the unicode hex code <CharHex> from the rlFNT\n"
		"    file <File>\n"
		"\n"
		"FontFace setstring <File> <StringName> \"<NewStringVal>\"\n"
		"    Change the value of the string <StringName> in rlFNT file <File> to <NewStringVal>\n"
		"    (Double quotes must be escaped via doubling them; only ASCII characters are \n"
		"    supported)\n"
		"    Available values for <StringName> (case sensitive):\n"
		"      Copyright     The copyright string of the font face\n"
		"                    Example: \"Copyright (c) 1982 Commodore International\"\n"
		"      FontFamily    The name of the font face's font family\n"
		"                    Example: \"C64\"\n"
		"      FontFace      The name of the font face\n"
		"                    (treated as suffix for FontFamily, if not empty)\n"
		"                    Example: \"Regular 40c\"\n"
		"\n"
		"FontFace validate <File>\n"
		"    Check if the rlFNT file <File> is valid\n"
		"\n"
		"\n"
	);
}


void WriteFontFaceError(rl::BitmapFont::Face::FileStatus status)
{
	using fs = rl::BitmapFont::Face::FileStatus;
	switch (status) // alphabetically sorted
	{
	case fs::DuplicateChar:
		rl::WriteError("Character defined multiple times");
		break;
	case fs::FileDoesntExist:
		rl::WriteError("File didn't exist");
		break;
	case fs::InvalidBPP:
		rl::WriteError("Invalid bits per pixel value");
		break;
	case fs::InvalidOffset_Copyright:
		rl::WriteError("Invalid offset to copyright string");
		break;
	case fs::InvalidOffset_Face:
		rl::WriteError("Invalid offset to face name");
		break;
	case fs::InvalidOffset_Family:
		rl::WriteError("Invalid offst to family name");
		break;
	case fs::InvalidWeight:
		rl::WriteError("Invalid weight value");
		break;
	case fs::NotAscending:
		rl::WriteError("Characters were not saved in ascending order");
		break;
	case fs::UnexpectedEOF:
		rl::WriteError("Unexpected end of file");
		break;
	case fs::UnknownFiletypeVer:
		rl::WriteError("Unknown filetype version");
		break;
	case fs::UnknownFlags:
		rl::WriteError("Unknown flags set");
		break;
	case fs::WrongMagicNo:
		rl::WriteError("Wrong magic number");
		break;
	case fs::ZeroHeight:
		rl::WriteError("Character height was set to zero");
		break;

	default:
		rl::WriteError("Unknown error");
	}
}


bool Decode(uint8_t ch, uint16_t codepage, uint32_t& result)
{
	char sz[2] = { (char)ch, 0 };
	wchar_t szw[3];
	int i = MultiByteToWideChar(codepage, 0, sz, 1, szw, 2);

	if (i == 0)
		return false;

	uint16_t iVal = szw[0];
	if (((iVal >> 8) & 0xFC) != 0b11011000) // single WORD
	{
		result = (uint32_t)iVal;
	}
	else // double WORD
	{
		uint16_t iVal2 = szw[1];
		if (((iVal2 >> 8) & 0xFC) != 0b11011100) // No low surrogate --> invalid encoding
			throw std::exception("Invalid UTF-16 encoding");

		unsigned int iResult = (iVal & 0x03FF) << 10;
		iResult |= (iVal2 & 0x03FF);
		iResult += 0x010000;

		result = iResult;
	}
	return true;
}


int CheckOutputPath(const wchar_t* szPath)
{
	DWORD dwLastError = 0;
	HANDLE hFile = CreateFileW(szPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, NULL,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		dwLastError = GetLastError();
		if (dwLastError != ERROR_FILE_EXISTS)
		{
			printf("Output path: ");
			rl::WriteWin32Error(dwLastError);
			return ERROR_BAD_PATHNAME;
		}
	}
	else
	{
		CloseHandle(hFile);
		DeleteFileW(szPath);
	}

	return ERROR_SUCCESS;
}


void DrawPixel(bool bVisible)
{
	rl::Console::SetCodepage(850);
	if (bVisible)
		printf("%c%c", 0xDB, 0xDB);
	else
		printf("  ");
	rl::Console::ResetCodepage();
}


bool UserConfirmation(bool bPrintValidInput)
{
	char ch = 0;
	do
	{
		ch = _getch();
		if (ch == VK_ESCAPE)
		{
			printf("\nExecution cancelled by the user\n\n\n");
			exit(ERROR_CANCELLED);
		}
	} while (ch != 'Y' && ch != 'y' && ch != 'N' && ch != 'n');

	ch = toupper(ch);

	if (bPrintValidInput)
		printf("%c\n", ch);

	return ch == 'Y';
}


void PrintChar(const rl::BitmapFont::Char& ch)
{
	if (!ch.hasData())
	{
		printf("[no graphics data]\n");
		return;
	}

	const char cOutline = '*';

	// single line box drawing characters, from CodePage 850
	const char cOutlineTL = (char)0xDA; // top-left corner
	const char cOutlineTR = (char)0xBF; // top-right corner
	const char cOutlineBL = (char)0xC0; // bottom-left corner
	const char cOutlineBR = (char)0xD9; // bottom-right corner
	const char cOutlineH = (char)0xC4; // horizontal line
	const char cOutlineV = (char)0xB3; // vertical line

	const char cCheckerBright = (char)0xB0; // bright background
	const char cCheckerDark = ' '; // dark background

	rl::Console::SetCodepage(850);



	printf("%c", cOutlineTL); // top-left corner

	// top line
	for (uint32_t i = 0; i < ch.getWidth(); i++)
		printf("%c%c", cOutlineH, cOutlineH);

	printf("%c\n", cOutlineTR); // top-right corner


	// pixel lines
	for (uint32_t iY = 0; iY < ch.getHeight(); iY++)
	{
		printf("%c", cOutlineV); // left line

		// draw the pixels
		for (uint32_t iX = 0; iX < ch.getWidth(); iX++)
		{
			if (ch.getPixel(iX, iY))
				printf("%c%c", 0xDB, 0xDB);
			else
			{
				if (((uint64_t)iX + iY) % 2)
					printf("%c%c", cCheckerBright, cCheckerBright);
				else
					printf("%c%c", cCheckerDark, cCheckerDark);
			}
		}

		printf("%c\n", cOutlineV); // right line
	}

	printf("%c", cOutlineBL); // bottom-left corner

	// bottom line
	for (uint32_t i = 0; i < ch.getWidth(); i++)
		printf("%c%c", cOutlineH, cOutlineH);

	printf("%c\n", cOutlineBR); // bottom-right corner



	rl::Console::ResetCodepage();
}


bool CheckFile(const wchar_t* szPath)
{
	auto status = rl::BitmapFont::Face::validate(szPath);
	if (status != rl::BitmapFont::Face::FileStatus::OK)
	{
		WriteFontFaceError(status);
		return false;
	}

	return true;
}


void UserMergeStringChoice(const char* szStringNameSingle, const char* szStringNameMultiple,
	const char* szCurrent, const char* szNew, std::string& res)
{
	// identical --> just keep the current one
	if (strcmp(szCurrent, szNew) == 0)
	{
		res = szCurrent;
		return;
	}

	// currently empty
	if (szNew[0] == 0)
	{
		printf("The %s is currently empty, but the new file has the value \"%s\"."
			"\nUse the new %s? [y/n] ", szStringNameSingle, szNew, szStringNameSingle);
		if (UserConfirmation(true))
			res = szNew;
		else
			res = szCurrent;
	}

	// currently set
	else
	{
		printf("The two files have different %s:\n"
			"Current: \"%s\"\n"
			"New:     \"%s\"\n"
			"\n"
			"Should the current value be replaced by the new one? [y/n] ",
			szStringNameMultiple, szCurrent, szNew);
		if (UserConfirmation(true))
			res = szNew;
		else
			res = szCurrent;
	}
}


void TrySaving(const rl::BitmapFont::Face& face, const wchar_t* path)
{
	if (!face.saveToFile(path))
	{
		bool bSaved = false;
		do
		{
			printf("File couldn't be saved to \"%ls\". Retry? [y/n] ", path);
			if (!UserConfirmation(true))
			{
				printf("File saving cancelled for now\n");
				break;
			}
			else
				bSaved = face.saveToFile(path);
		} while (!bSaved);
	}
	else
		printf("Saved file\n");
}
