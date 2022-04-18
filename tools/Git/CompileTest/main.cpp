#include "rl/console.hpp"

#include <Windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#include <regex>
#include <vector>





// computer dependent path for now
const wchar_t szMSBuildEXE[] = LR"(D:\Programme\Microsoft Visual Studio\2019\MSBuild\Current\Bin\MSBuild.exe)";
const wchar_t szProjectExt[] = L".vcxproj";



void TestCompile(const wchar_t* szPath, const wchar_t* szLogFilePath);



int wmain(int argc, wchar_t* argv[])
{
	wchar_t szLogFile[MAX_PATH + 1] = {}; // the log file that should be used for storing data
	{
		wchar_t szLogFileEnvVar[MAX_PATH + 1] = {};
		swprintf_s(szLogFileEnvVar, LR"(%%TEMP%%\RobinLe_CT_%d.log)", GetCurrentProcessId());
		ExpandEnvironmentStringsW(szLogFileEnvVar, szLogFile, MAX_PATH + 1);
	}

	if (argc == 1)
	{
		printf("Please provide at least one path to check (either a folder or a .sln file)\n");
		return ERROR_BAD_COMMAND;
	}


	for (unsigned i = 1; i < (unsigned)argc; i++)
		TestCompile(argv[i], szLogFile);


	return ERROR_SUCCESS;
}


using con = rl::Console;





void TestCompile(const wchar_t* szPath, const wchar_t* szLogFilePath)
{
	DWORD dwAttribs = GetFileAttributesW(szPath);

	// could not get attributes --> Exit
	if (dwAttribs == INVALID_FILE_ATTRIBUTES)
	{
		con::PushColor(FG_DARKRED);
		printf("Could not process path \"%ls\"\n", szPath);
		con::PopColor();
		return;
	}

	// directory --> recursive
	if (dwAttribs & FILE_ATTRIBUTE_DIRECTORY)
	{
		wchar_t szSubPath[MAX_PATH + 1];
		wcscpy_s(szSubPath, szPath);
		PathAddBackslashW(szSubPath);
		wchar_t szFindResult[MAX_PATH + 1];
		wcscpy_s(szFindResult, szSubPath);

		wcscat_s(szSubPath, L"*");

		const size_t iFindOffset = wcslen(szSubPath) - 1;



		// search for sub-directories

		WIN32_FIND_DATAW oFind;
		HANDLE hFind = FindFirstFileW(szSubPath, &oFind);

		if (hFind == INVALID_HANDLE_VALUE)
			return; // no subdirectories or files
		do
		{
			// skip files
			if ((oFind.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
				continue;

			// skip "." and ".." pseudo-directories
			if (wcscmp(oFind.cFileName, L".") == 0 || wcscmp(oFind.cFileName, L"..") == 0)
				continue;

			wcscat_s(szFindResult, oFind.cFileName);
			TestCompile(szFindResult, szLogFilePath);
			szFindResult[iFindOffset] = 0;
		} while (FindNextFileW(hFind, &oFind));
		FindClose(hFind);



		// search for solution files

		wcscat_s(szSubPath, szProjectExt);
		hFind = FindFirstFileW(szSubPath, &oFind);
		if (hFind == INVALID_HANDLE_VALUE)
			return; // no .vcxproj files
		do
		{
			szFindResult[iFindOffset] = 0;
			wcscat_s(szFindResult, oFind.cFileName);
			TestCompile(szFindResult, szLogFilePath);
		} while (FindNextFileW(hFind, &oFind));
	}

	// file
	else
	{
		const wchar_t* pExt = wcsrchr(szPath, L'.');
		if (!pExt || _wcsicmp(pExt, szProjectExt) != 0) // case insensitive extension comparison
		{
			con::PushColor(FG_YELLOW);
			printf("Ignored file \"%ls\" because it was no %ls file\n", szPath, szProjectExt);
			con::PopColor();
			return;
		}

		// first number: required characters for options and quotes
		wchar_t szParams[126 + MAX_PATH + MAX_PATH + 1] = {};

		swprintf_s(szParams, L"\"%ls\" -t:Rebuild -p:Configuration=Debug -p:Platform=x64 "
			L"-fileLogger -fileLoggerParameters:Verbosity=q;Encoding=Unicode;LogFile=\"%ls\"",
			szPath, szLogFilePath);


		SHELLEXECUTEINFOW ShExecInfo = { sizeof(SHELLEXECUTEINFOW) };
		ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		ShExecInfo.hwnd = NULL;
		ShExecInfo.lpVerb = NULL;
		ShExecInfo.lpFile = szMSBuildEXE;
		ShExecInfo.lpParameters = szParams;
		ShExecInfo.lpDirectory = NULL;
		ShExecInfo.nShow = SW_HIDE;
		ShExecInfo.hInstApp = NULL;

		ShellExecuteExW(&ShExecInfo);
		if (ShExecInfo.hProcess != 0)
		{
			WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
			CloseHandle(ShExecInfo.hProcess);
		}


		size_t lenData = 0;
		wchar_t* pData = nullptr;

		HANDLE hFile = CreateFileW(szLogFilePath, GENERIC_READ, FILE_SHARE_READ, NULL,
			OPEN_EXISTING, NULL, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			LARGE_INTEGER liFileSize;
			GetFileSizeEx(hFile, &liFileSize);
			lenData = liFileSize.QuadPart / 2;

			pData = new wchar_t[liFileSize.QuadPart];

			uint64_t iOffset = 0;
			DWORD dwRead = 0;

			while (liFileSize.QuadPart - iOffset > MAXDWORD)
			{
				if (!ReadFile(hFile, pData + (iOffset / 2), MAXDWORD, &dwRead, NULL))
				{
					CloseHandle(hFile);
					DeleteFileW(szLogFilePath);
					delete[] pData;
					return;
				}

				iOffset += MAXDWORD;
			}

			if (!ReadFile(hFile, pData + (iOffset / 2), (DWORD)(liFileSize.QuadPart - iOffset),
				&dwRead, NULL))
			{
				CloseHandle(hFile);
				DeleteFileW(szLogFilePath);
				delete[] pData;
				return;
			}

			CloseHandle(hFile);
		}


		DeleteFileW(szLogFilePath);


		std::vector<const wchar_t*> oLines;
		if (lenData > 1)
		{
			size_t iOffset = 1; // skip BOM
			do
			{
				oLines.push_back(pData + iOffset);

				while (iOffset < lenData && pData[iOffset] != L'\r')
					++iOffset;

				if (pData[iOffset] == L'\r')
				{
					pData[iOffset] = 0;
					iOffset += 2; // skip "\r\n"
				}
			} while (iOffset < lenData);
		}

		enum class ErrorType
		{
			Warning,
			Error
		};

		struct ErrorLogEntry
		{
			std::wstring sSourceFile;
			uint64_t iLine;
			uint64_t iChar;
			ErrorType eType;
			char szCode[6];
			std::wstring sMessage;
		};

		std::vector<ErrorLogEntry> oEntries;

		for (auto szLine : oLines)
		{
			std::wregex regex(LR"(^(.*)\((\d+),(\d+)\): (error|warning) ([A-Z]\d{4}|): (.+) \[.+\.vcxproj\]$)");
			std::wcmatch matches;
			std::regex_search(szLine, matches, regex);

			if (matches.ready())
			{
				ErrorLogEntry entry = {};
				entry.sSourceFile = matches.str(1);
				entry.iLine = std::stoull(matches.str(2));
				entry.iChar = std::stoull(matches.str(3));
				if (matches.str(4) == L"error")
					entry.eType = ErrorType::Error;
				else
					entry.eType = ErrorType::Warning;

				std::wstring sCode = matches.str(5);
				if (sCode.empty())
					entry.szCode[0] = 0;
				else
				{
					entry.szCode[5] = 0;
					for (uint8_t i = 0; i < 5; i++)
						entry.szCode[i] = (char)sCode[i];
				}

				entry.sMessage = matches.str(6);

				oEntries.push_back(entry);
			}
		}


		if (oEntries.size() == 0)
		{
			con::PushColor(FG_DARKGREEN);
			printf("Success:\t");
			con::PopColor();
			printf("%ls\n\n", szPath);
			return;
		}

		ErrorType eHighestError = ErrorType::Warning;
		for (auto& o : oEntries)
		{
			if (o.eType == ErrorType::Error)
			{
				eHighestError = ErrorType::Error;
				break;
			}
		}

		switch (eHighestError)
		{
		case ErrorType::Error:
			con::PushColor(FG_DARKRED);
			printf("Error:\t");
			break;
		case ErrorType::Warning:
			con::PushColor(FG_YELLOW);
			printf("Warning:\t");
			break;
		}
		con::PopColor();

		printf("%ls\n", szPath);

		for (auto& o : oEntries)
		{
			printf("\t");
			switch (o.eType)
			{
			case ErrorType::Error:
				con::PushColor(FG_DARKRED);
				printf("Error");
				break;
				
			case ErrorType::Warning:
				con::PushColor(FG_YELLOW);
				printf("Warning");
				break;
			}

			if (strlen(o.szCode) > 0)
			{
				printf(" %s", o.szCode);
			}
			con::PopColor();
			printf(": %ls\n", o.sMessage.c_str());
		}

		printf("\n");
	}
}