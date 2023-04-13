#include "rl/console.hpp"

// rl
#include <rl/dev.msbuild.hpp>

// Win32
#include <Windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

// STL
#include <regex>
#include <vector>





// computer dependent path for now
const wchar_t szMSBuildEXE[] = LR"(D:\Programme\Microsoft Visual Studio\2019\MSBuild\Current\Bin\MSBuild.exe)";
const wchar_t szProjectExt[] = L".vcxproj";



void TestCompile(const wchar_t* szPath);



int wmain(int argc, wchar_t* argv[])
{
	if (argc == 1)
	{
		std::printf("Please provide at least one path to check (either a folder or a .sln file)\n");
		return ERROR_BAD_COMMAND;
	}

	wchar_t szPath[MAX_PATH + 1];
	for (unsigned i = 1; i < (unsigned)argc; i++)
	{
		szPath[0] = L'\0';
		// expand environment variables
		if (!ExpandEnvironmentStringsW(argv[i], szPath, MAX_PATH + 1) || szPath[0] == '\0')
		{
			std::printf("Ignoring \"%ls\".\n\n", argv[i]);
			continue;
		}

		std::printf("Processing path \"%ls\"...\n", szPath);
		TestCompile(szPath);
	}


	return ERROR_SUCCESS;
}


using con = rl::Console;





void TestCompile(const wchar_t* szPath)
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
			TestCompile(szFindResult);
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
			TestCompile(szFindResult);
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

		std::printf("Compiling \"%ls\"...\n", szPath);

		auto result = rl::Compiler::Compile(szPath);

		if (!result)
		{
			con::PushColor(FG_RED);
			printf("Compilation failed: %s.\n\n", result.errorMessage().c_str());
			con::PopColor();
			return;
		}

		if (result.projects().size() == 0)
		{
			con::PushColor(FG_GREEN);
			printf("Successfully compiled \"%ls\".\n\n", szPath);
			con::PopColor();
			return;
		}

		for (auto &it : result.projects())
		{
			const auto &sProject = it.first;
			const auto &oProject = it.second;

			const auto iErrors   = oProject.errorCount();
			const auto iWarnings = oProject.warningCount();

			if (iErrors == 0 && iWarnings == 0)
			{
				con::PushColor(FG_GREEN);
				printf("Project \"%ls\" compiled successfully.\n\n", sProject.c_str());
				con::PopColor();
			}
			else
			{
				uint8_t iColor;
				if (iErrors == 0)
					iColor = FG_YELLOW;
				else
					iColor = FG_RED;

				con::PushColor(iColor);
				printf("Project \"%ls\" compiled with %zu errors and %zu warnings.\n",
					sProject.c_str(), iErrors, iWarnings);
				if (iErrors)
				{
					std::printf("Errors:\n");
					for (const auto &oError : oProject.items())
					{
						if (oError.eType == rl::CompileMessageType::Error)
							std::printf("  \"%ls\" (%zu): %ls\n",
								oError.sFile.c_str(), oError.iLine, oError.sDescription.c_str());
					}
					con::PopColor();
					con::PushColor(FG_YELLOW);
				}
				if (iWarnings)
				{
					std::printf("Warnings:\n");
					for (const auto &oError : oProject.items())
					{
						if (oError.eType == rl::CompileMessageType::Warning)
							std::printf("  \"%ls\" (%zu): %ls\n",
								oError.sFile.c_str(), oError.iLine, oError.sDescription.c_str());
					}
				}

				con::PopColor();

				std::printf("\n");
			}
		}

		printf("\n");
	}
}