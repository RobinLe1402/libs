#include "rl/text.fileio.hpp"

#include <Windows.h>

int wmain(int argc, wchar_t* argv[])
{
	// write test
	if (false)
	{
		rl::TextFileWriter writer;
		writer.open(LR"(E:\[TempDel]\output.txt)", rl::TextEncoding::UTF8BOM, rl::LineBreak::UNIX);
		writer.writeLine(L"Zeile 1 :D");
		writer.write(L"Zeile 2 ;-)");
		writer.close();
		return 0;
	}


	// encoding guess test
	if (false)
	{
		if (argc == 1)
		{
			printf("Please add a text file path as a parameter.\n");
			return 1;
		}

		rl::TextEncoding encoding;
		if (!rl::GuessTextEncoding(argv[1], encoding, false))
		{
			printf("Couldn't guess the encoding\n");
			return 1;
		}

		switch (encoding)
		{
		case rl::TextEncoding::CP1252:
			printf("Codepage 1252 (\"ANSI\")\n");
			break;
		case rl::TextEncoding::UTF8:
			printf("UTF-8 (without BOM)\n");
			break;
		case rl::TextEncoding::UTF8BOM:
			printf("UTF-8 (with BOM)\n");
			break;
		case rl::TextEncoding::UTF16BE:
			printf("UTF-16 (big endian)\n");
			break;
		case rl::TextEncoding::UTF16LE:
			printf("UTF-16 (little endian)\n");
			break;
		case rl::TextEncoding::UTF32BE:
			printf("UTF-32 (big endian)\n");
			break;
		case rl::TextEncoding::UTF32LE:
			printf("UTF-32 (little endian)\n");
			break;
		}

	}

	
	// read + write test
	if (true)
	{
		std::vector<std::wstring> oLines;
		rl::ReadAllLines(LR"(E:\[Temp]\test.txt)", oLines);
		rl::WriteTextFile(LR"(E:\[TempDel]\output.txt)", oLines);
	}


	return 0;
}
