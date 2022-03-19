#include "rl/text.fileio.hpp"

#include <Windows.h>

int wmain(int argc, wchar_t* argv[])
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
		if (argc == 1)
		{
			printf("Please add a text file path as a parameter.\n");
			return 1;
		}

		rl::TextFileInfo tfi{};
		rl::TextFileInfo_Get tfiEx{};
		if (!rl::GetTextFileInfo(argv[1], tfi, tfiEx))
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
			const char* szEndian = (tfi.iFlags & rl::Flags::TextFileInfo::BigEndian) ?
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


	return 0;
}
