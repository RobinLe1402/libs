#include "tests.hpp"

// rl
#include <rl/text.fileio.hpp>

// Win32
#include <Windows.h>

// STL
#include <iostream>



bool UnitTest_text_fileio()
{
	// write test
	if constexpr (false)
	{
		rl::TextFileWriter writer;
		writer.open(LR"(E:\[TempDel]\output.txt)", rl::TextFileInfo_UTF8BOM(rl::LineBreak::UNIX));
		writer.writeLine(L"Zeile 1 :D");
		writer.write(L"Zeile 2 ;-)");
		writer.close();
		return 0;
	}


	// encoding guess test
	if constexpr (true)
	{
		std::wstring sFile;

		bool bValid;
		do
		{
			bValid = true;

			std::wcout << L"Please enter the path of a text file: ";
			std::wcin >> sFile;
			if (std::wcin.fail())
			{
				const DWORD dwAttr = GetFileAttributesW(sFile.c_str());
				if (dwAttr == INVALID_FILE_ATTRIBUTES || (dwAttr & FILE_ATTRIBUTE_DIRECTORY))
				{
					std::wcout << L"Invalid file path.\n\n";
					bValid = false;
				}
			}
		} while (!bValid);

		rl::TextFileInfo tfi{};
		rl::TextFileInfo_Get tfiEx{};
		if (!rl::GetTextFileInfo(sFile.c_str(), tfi, tfiEx))
		{
			printf("Couldn't guess the encoding\n");
			return 1;
		}

		// output encoding
		switch (tfi.eEncoding)
		{
		case rl::TextEncoding::ASCII:
			printf("ASCII\n");
			break;
		case rl::TextEncoding::Codepage:
			printf("Codepage\n");
			break;

		case rl::TextEncoding::UTF8:
			printf("UTF-8");
			if (tfi.iFlags & rl::Flags::TextFileInfo::HasBOM)
				printf(" BOM");
			printf("\n");
			break;

		default: // UTF-16 or UTF-32
		{
			switch (tfi.eEncoding)
			{
			case rl::TextEncoding::UTF16:
				printf("UTF-16");
				break;
			case rl::TextEncoding::UTF32:
				printf("UTF-32");
				break;
			}

			const char szBigEndian[] = "big endian";
			const char szLittleEndian[] = "little endian";
			const char *szEndian = (tfi.iFlags & rl::Flags::TextFileInfo::BigEndian) ?
				szBigEndian : szLittleEndian;

			printf(" (%s)\n", szEndian);
		}
		}

		// output linebreaks
		switch (tfi.eLineBreaks)
		{
		case rl::LineBreak::Windows:
			printf("Windows linebreaks (\\r\\n)\n");
			break;
		case rl::LineBreak::UNIX:
			printf("UNIX linebreaks (\\n)\n");
			break;
		case rl::LineBreak::Macintosh:
			printf("Macintosh linebreaks (\\r)\n");
			break;
		}

	}


	// read + write test
	if constexpr (false)
	{
		std::vector<std::wstring> oLines;
		rl::ReadAllLines(LR"(E:\[Temp]\test.txt)", oLines);
		rl::WriteTextFile(LR"(E:\[TempDel]\output_ascii.txt)", oLines, rl::TextFileInfo_ASCII());
		rl::WriteTextFile(LR"(E:\[TempDel]\output_codepage.txt)", oLines, rl::TextFileInfo_Codepage());
		rl::WriteTextFile(LR"(E:\[TempDel]\output_utf8.txt)", oLines, rl::TextFileInfo_UTF8());
		rl::WriteTextFile(LR"(E:\[TempDel]\output_utf8bom.txt)", oLines, rl::TextFileInfo_UTF8BOM());
		rl::WriteTextFile(LR"(E:\[TempDel]\output_utf16le.txt)", oLines, rl::TextFileInfo_UTF16LE());
		rl::WriteTextFile(LR"(E:\[TempDel]\output_utf16be.txt)", oLines, rl::TextFileInfo_UTF16BE());
		rl::WriteTextFile(LR"(E:\[TempDel]\output_utf32le.txt)", oLines, rl::TextFileInfo_UTF32LE());
		rl::WriteTextFile(LR"(E:\[TempDel]\output_utf32be.txt)", oLines, rl::TextFileInfo_UTF32BE());
	}


	return true;
}