#include "include/main.hpp"

#include <conio.h>
#include <string>
#include <Shlwapi.h>
#include <Windows.h>

#include "rl/text.fileio.hpp"


#pragma comment(lib, "Shlwapi.lib")



int wmain(int argc, wchar_t* argv[])
{
	SetConsoleTitleW(L"RobinLe Documentation Generator     |     \u00A9 2022 RobinLe");

	std::wstring sOutDir;
	bool bArg_O = false;
	for (int iArg = 1; iArg < argc; ++iArg)
	{
		if (argv[iArg][0] != '/')
			continue;

		std::wstring_view sArg(argv[iArg] + 1);

		// single character parameter with value
		if (sArg.length() > 2 && sArg[1] == ':')
		{
			wchar_t cArg = sArg[0];

			switch (cArg)
			{
			case 'o': // fallthrough: not case sensitive
			case 'O':
				if (bArg_O)
				{
					printf("Please only define one output directory.\n");
					return 1; // multiple /O parameters
				}
				sOutDir = sArg.substr(2);
				bArg_O = true;
				break;

			default: // unkown parameter character
				PrintUsage();
				return 1;
			}
		}
	}

	//==============================================================================================
	// check output path

	if (!bArg_O)
	{
		PrintUsage();
		return 1; // output path missing from arguments
	}

	if (sOutDir[0] == '"')
	{
		if (sOutDir.length() < 3 || !sOutDir.ends_with('"'))
		{
			printf("Illegal output path.\n");
			return 1; // path started with ", but didn't end with "
		}

		sOutDir.erase(sOutDir.begin());
		sOutDir.pop_back();

		if (sOutDir.ends_with('\\') || sOutDir.ends_with('/'))
			sOutDir.pop_back(); // remove trailing delimiter
	}

	DWORD dwFileAttribs = GetFileAttributesW(sOutDir.c_str());
	if (dwFileAttribs == INVALID_FILE_ATTRIBUTES)
	{
		switch (GetLastError())
		{
		case ERROR_FILE_NOT_FOUND:
		{
			if (!CreateDirectoryW(sOutDir.c_str(), NULL))
			{
				printf("Couldn't create output directory \"%ls\"\n", sOutDir.c_str());
				return 1; // CreateDirectoryW failed
			}
			break;
		}

		case ERROR_ACCESS_DENIED:
			printf("Access to output directory denied\n");
			return 1;

		default: // other error
			printf("Can't access output directory (Error #%u)\n", GetLastError());
			return 1;
		}
	}
	else if ((dwFileAttribs & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		printf("Please specify a directory as output path.\n");
		return 1; // output path was no directory
	}
	else if (!PathIsDirectoryEmptyW(sOutDir.c_str()))
	{
		printf("Warning: Destination directory is not empty. Continue?");
		if (!GetUserConfirmation())
			return 0; // canceled by user
	}






	//==============================================================================================
	// get, check value of %GitHub_rl_libs%

	std::wstring sGitRoot(MAX_PATH, 0);
	{
		const wchar_t szEnvVar[] = L"GitHub_rl_libs";

		const DWORD dwResult =
			GetEnvironmentVariableW(szEnvVar, (LPWSTR)sGitRoot.c_str(),
				(DWORD)sGitRoot.length() + 1);
		if (dwResult == 0)
		{
			printf("Environment variable %%%ls%% was undefined.\n", szEnvVar);
			return 1;
		}
		if (dwResult > sGitRoot.length() + 1)
		{
			printf("Value of %%%ls%% was too long (maximum length is %u).\n", szEnvVar, MAX_PATH);
			return 1;
		}

		dwFileAttribs = GetFileAttributesW(sGitRoot.c_str());
		if (dwFileAttribs == INVALID_FILE_ATTRIBUTES)
		{
			switch (GetLastError())
			{
			case ERROR_FILE_NOT_FOUND:
				printf("Directory \"%ls\" was not found.\n", sGitRoot.c_str());
				return 1;

			case ERROR_ACCESS_DENIED:
				printf("Access to \"%ls\" was denied.\n", sGitRoot.c_str());
				return 1;

			default:
				printf("Couldn't access \"%ls\" (error #%u).\n", sGitRoot.c_str(), GetLastError());
				return 1;
			}
		}
		if ((dwFileAttribs & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{
			printf("Value of environment variable %%%ls%% (\"%ls\") is no directory.\n",
				szEnvVar, sGitRoot.c_str());
			return 1;
		}
	}

	printf("Input: \"%ls\"\nOutput: \"%ls\"\n", sGitRoot.c_str(), sOutDir.c_str());

	// todo: parse source files, extract XML, generate HTML




	return 0;
}

void PrintUsage()
{
	printf(
		"Usage:  DocGen /O:OutputPath\n" // ToDo: "[/C|/H|/X]" --> CHM/HTML/XML
		"\n"
		"  /O          Output path\n"
	);
}

bool GetUserConfirmation()
{
	printf(" [Y/N] ");
	char c = 0;
	do
	{
		c = _getch();
		if (iscntrl(c))
		{
			switch (c)
			{
			case L'\u001B': // [ESC] --> "N"
				c = 'N';
				break;

			default:
				continue;
			}
		}

		if (c < 'A' || c > 'z')
			continue; // cannot be used in a call to islower()
		if (islower(c))
			c = toupper(c);
	} while (c != 'Y' && c != 'N');

	printf("%c\n", c);

	return (c == 'Y');
}
