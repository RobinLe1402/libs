#include <cstdio>
#include <map>
#include <stdint.h>
#include <Windows.h>





bool ReadStr(HANDLE hFile, char** dest);
bool SkipVal(HANDLE hFile);

bool WriteStr(HANDLE hFile, const char* sz);



int wmain(int argc, wchar_t* argv[])
{
	if (argc != 3)
	{
		printf("Please use the following syntax:\n"
			"\tUnicodeData_Generator <UnicodeData.txt> <OutputFile.cpp>\n\n");
		return 1;
	}

	const wchar_t* szUnicodeDataTXT = argv[1];
	const wchar_t* szOutputCPP = argv[2];



	//==============================================================================================
	// PARSE INPUT FILE

	HANDLE hFile = CreateFileW(szUnicodeDataTXT, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
		NULL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("Error while opening input file \"%ls\"\n\n", szUnicodeDataTXT);
		return 1;
	}

	uint64_t iFileSize;
	{
		LARGE_INTEGER liFileSize;
		GetFileSizeEx(hFile, &liFileSize);
		iFileSize = liFileSize.QuadPart;
	}

	LARGE_INTEGER liFilePtr;

	std::map<uint32_t, const char*> oNames;

	printf("Parsing unicode data... ");
	while (SetFilePointerEx(hFile, { 0 }, &liFilePtr, FILE_CURRENT),
		(uint64_t)liFilePtr.QuadPart < iFileSize)
	{
		DWORD dwRead = 0;
		char ch;

		char szHexVal[7] = {};
		uint32_t iBinVal = 0;
		for (uint8_t i = 0; i < 7; ++i)
		{
			if (!ReadFile(hFile, &ch, 1, &dwRead, NULL))
				return 1; // read error

			if (ch == ';')
			{
				szHexVal[i] = 0;
				break;
			}
			else if (i == 6)
				return 1; // hex number was too long
			else
				szHexVal[i] = ch;
		}
		iBinVal = strtoul(szHexVal, nullptr, 16); // convert hex string to binary value


		char* szName = nullptr;
		if (!ReadStr(hFile, &szName))
		{
			CloseHandle(hFile);
			return 1; // couldn't read current name
		}

		char* szNameUnicode1 = nullptr;

		// skip values not interested in
		for (uint8_t i = 0; i < 8; ++i)
		{
			if (!SkipVal(hFile))
			{
				CloseHandle(hFile);
				return 1; // couldn't skip value
			}
		}

		if (!ReadStr(hFile, &szNameUnicode1))
		{
			CloseHandle(hFile);
			return 1; // couldn't read Unicode 1.0 name
		}


		{
			char ch = 0;
			while (ch != '\n')
			{
				if (!ReadFile(hFile, &ch, 1, &dwRead, NULL))
				{
					CloseHandle(hFile);
					return 1; // couldn't read until end of line
				}
			}
		}



		// decide about string to use
		const char* szNameFinal = szName;
		if (strlen(szName) == 0 || ((szName[0] == '<') && (strlen(szNameUnicode1) > 0)))
			szNameFinal = szNameUnicode1;

		if (szNameFinal == szName)
			delete[] szNameUnicode1;
		else
			delete[] szName;


		oNames[iBinVal] = szNameFinal;
	}
	CloseHandle(hFile);

	printf("Done.\n");




	//==============================================================================================
	// WRITE CPP FILE

	hFile = CreateFileW(szOutputCPP, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("Could not create output file \"%ls\"\n\n", szOutputCPP);
		return 1;
	}

	printf("Writing file... ");

#define WRITE_STRING(s) if (!WriteStr(hFile, s)) { CloseHandle(hFile); return 1; }
#define ENDL "\r\n"

	char szNumber[7] = {}; // codepoint value as hex string
	char szLen[5] = {}; // length of the name

	WRITE_STRING("/*"																			ENDL);
	WRITE_STRING("This file is created automatically from UnicodeData.txt"						ENDL);
	WRITE_STRING("*/"																			ENDL);
	WRITE_STRING(""																				ENDL);
	WRITE_STRING(""																				ENDL);
	WRITE_STRING("#define LIBRARY_EXPORTS"														ENDL);
	WRITE_STRING(""																				ENDL);
	WRITE_STRING("#include <rl/dll/UnicodeData.hpp>" 											ENDL);
	WRITE_STRING("#include <string.h> // strcpy_s" 												ENDL);
	WRITE_STRING("#include <stdint.h>"															ENDL);
	WRITE_STRING(""																				ENDL);
	WRITE_STRING(""																				ENDL);
	WRITE_STRING("namespace rl" 																ENDL);
	WRITE_STRING("{"																			ENDL);
	WRITE_STRING("	namespace UnicodeDataDLL" 													ENDL);
	WRITE_STRING("	{" 																			ENDL);
	WRITE_STRING("		NameLen_t __stdcall GetNameLen(UChar_t ch)"								ENDL);
	WRITE_STRING("		{"																		ENDL);
	WRITE_STRING("			switch (ch)" 														ENDL);
	WRITE_STRING("			{" 																	ENDL);
	for (auto& o : oNames)
	{
		// convert codepoint to string
		sprintf_s(szNumber, "%X", o.first);

		// convert length to string
		sprintf_s(szLen, "%X", (uint32_t)strlen(o.second) + 1);

		// write case
		WRITE_STRING("			case 0x");
		WRITE_STRING(szNumber);
		WRITE_STRING(":" ENDL);
		WRITE_STRING("				return 0x");
		WRITE_STRING(szLen);
		WRITE_STRING(";" ENDL);
	}
	WRITE_STRING("			"																	ENDL);
	WRITE_STRING("			default: // not defined" 											ENDL);
	WRITE_STRING("				return 0;"														ENDL);
	WRITE_STRING("			}"																	ENDL);
	WRITE_STRING("		}"																		ENDL);
	WRITE_STRING("		"																		ENDL);
	WRITE_STRING("		NameLen_t __stdcall GetName(UChar_t ch, char* buf, NameLen_t buf_size)"	ENDL);
	WRITE_STRING("		{"																		ENDL);
	WRITE_STRING("			const NameLen_t len = GetNameLen(ch);"								ENDL);
	WRITE_STRING("			if (len == 0 || len > buf_size)"									ENDL);
	WRITE_STRING("				return 0;"														ENDL);
	WRITE_STRING("			" 																	ENDL);
	WRITE_STRING("			switch (ch)" 														ENDL);
	WRITE_STRING("			{" 																	ENDL);
	for (auto& o : oNames)
	{
		// convert codepoint to string
		sprintf_s(szNumber, "%X", o.first);

		// write case
		WRITE_STRING("			case 0x");
		WRITE_STRING(szNumber);
		WRITE_STRING(":" ENDL);
		WRITE_STRING("				strcpy_s(buf, buf_size, \"");
		WRITE_STRING(o.second);
		WRITE_STRING("\");" ENDL);
		WRITE_STRING("				break;" ENDL);
	}
	WRITE_STRING("			"																	ENDL);
	WRITE_STRING("			default: // not defined" 											ENDL);
	WRITE_STRING("				return 0;" 														ENDL);
	WRITE_STRING("			}" 																	ENDL);
	WRITE_STRING("			" 																	ENDL);
	WRITE_STRING("			return (NameLen_t)strlen(buf);"										ENDL);
	WRITE_STRING("		}" 																		ENDL);
	WRITE_STRING("	}" 																			ENDL);
	WRITE_STRING("}" 																			ENDL);

#undef WRITE_STRING
#undef ENDL

	CloseHandle(hFile);
	printf("Done.\n\nFile \"%ls\" was successfully generated\n\n", szOutputCPP);

	return 0;
}


bool ReadStr(HANDLE hFile, char** dest)
{
	LARGE_INTEGER liFilePtr;
	SetFilePointerEx(hFile, { 0 }, &liFilePtr, FILE_CURRENT);

	char ch = 0;
	DWORD dwRead = 0;

	// 1. count the characters
	uint16_t len = 0;
	while (ch != ';')
	{
		if (!ReadFile(hFile, &ch, 1, &dwRead, NULL))
			return false;

		++len;
	}

	*dest = new char[len];
	(*dest)[len - 1] = 0; // terminating zero

	SetFilePointerEx(hFile, liFilePtr, NULL, FILE_BEGIN); // reset to start of value

	// 2. read the characters
	if (!ReadFile(hFile, *dest, len - 1, &dwRead, NULL))
	{
		delete[](*dest);
		*dest = nullptr;
		return false;
	}
	SetFilePointer(hFile, 1, NULL, FILE_CURRENT); // skip ";"

	return true;
}

bool SkipVal(HANDLE hFile)
{
	char ch = 0;
	DWORD dwRead = 0;

	while (ch != ';')
		if (!ReadFile(hFile, &ch, 1, &dwRead, NULL))
			return false;

	return true;
}

bool WriteStr(HANDLE hFile, const char* sz)
{
	const size_t len = strlen(sz);
	DWORD dwWritten = 0;
	if (!WriteFile(hFile, sz, (DWORD)len, &dwWritten, NULL) || dwWritten != len)
		return false;

	return true;
}
